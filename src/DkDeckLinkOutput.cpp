#include "DkDeckLinkOutput.h"
#include "DkDeckLinkPlatform.h"
#include "DkDeckLinkCompat.h"

#ifdef _WIN32
// midl-generated DeckLinkAPI.h does not declare these; DeckLinkAPIDispatch.cpp defines them.
IDeckLinkIterator* CreateDeckLinkIteratorInstance();
IDeckLinkAPIInformation* CreateDeckLinkAPIInformationInstance();
IDeckLinkVideoConversion* CreateVideoConversionInstance();
IDeckLinkDiscovery* CreateDeckLinkDiscoveryInstance();
#endif

#include <QDebug>
#include <algorithm>
#include <cstring>

// On Mac, REFIID is CFUUIDBytes — no built-in operator==, so use memcmp.
static inline bool refiidEqual(REFIID a, REFIID b)
{
    return std::memcmp(&a, &b, sizeof(REFIID)) == 0;
}

namespace nmc
{

// -----------------------------------------------------------------------
// Internal helpers
// -----------------------------------------------------------------------

static long computeRowBytes(long width, BMDPixelFormat fmt)
{
    switch (fmt) {
    case bmdFormat8BitBGRA:    return width * 4;
    case bmdFormat10BitRGB:    return width * 4;           // 1 32-bit word per pixel
    case bmdFormat12BitRGB:
    case bmdFormat12BitRGBLE:  return ((width + 1) / 2) * 9; // 2 pixels per 9-byte block
    case bmdFormat8BitYUV:     return ((width + 1) / 2) * 4; // Cb+Y0+Cr+Y1 per pair
    case bmdFormat10BitYUV:    return ((width + 47) / 48) * 128; // v210: ceil(w/48)*128 bytes
    default:                   return width * 4;
    }
}

static const QVector<DkPixelFormatInfo> kAllFormats = {
    { bmdFormat8BitBGRA,   QStringLiteral("8-bit RGBA")              },
    { bmdFormat10BitRGB,   QStringLiteral("10-bit RGB (r210)")        },
    { bmdFormat12BitRGB,   QStringLiteral("12-bit RGB (R12B)")        },
    { bmdFormat12BitRGBLE, QStringLiteral("12-bit RGB LE (R12L)")     },
    { bmdFormat8BitYUV,    QStringLiteral("8-bit YUV 4:2:2 (2vuy)")  },
    { bmdFormat10BitYUV,   QStringLiteral("10-bit YUV 4:2:2 (v210)") },
};

// -----------------------------------------------------------------------
// SDK version compatibility helpers
//
// SDK 16.0 changed the IIDs for IDeckLinkOutput, IDeckLinkConfiguration,
// IDeckLinkProfileAttributes, and IDeckLinkVideoBuffer.  On DeckLink Desktop
// Video 15.x the 16.0 QueryInterface fails; we fall back to the 15.x IIDs.
// On 15.1 and earlier there is no IDeckLinkVideoBuffer at all — GetBytes
// lived directly on IDeckLinkVideoFrame (IDeckLinkVideoFrame_v14_2_1).
// -----------------------------------------------------------------------

// RAII wrapper that locks a frame's write buffer, handling all three SDK eras.
struct FrameWriteBuffer {
    void                        *buf     = nullptr;
    IDeckLinkVideoBuffer        *vbuf    = nullptr;
    IDeckLinkVideoBuffer_v15_3_1 *vbuf153 = nullptr;
    IDeckLinkVideoFrame_v14_2_1  *old     = nullptr;

    explicit FrameWriteBuffer(IDeckLinkMutableVideoFrame *frame) {
        if (frame->QueryInterface(IID_IDeckLinkVideoBuffer,
                                   reinterpret_cast<void **>(&vbuf)) == S_OK) {
            vbuf->StartAccess(bmdBufferAccessWrite);
            vbuf->GetBytes(&buf);
        } else if (frame->QueryInterface(IID_IDeckLinkVideoBuffer_v15_3_1,
                                          reinterpret_cast<void **>(&vbuf153)) == S_OK) {
            vbuf153->StartAccess(bmdBufferAccessWrite);
            vbuf153->GetBytes(&buf);
        } else if (frame->QueryInterface(IID_IDeckLinkVideoFrame_v14_2_1,
                                          reinterpret_cast<void **>(&old)) == S_OK) {
            old->GetBytes(&buf);
        }
    }

    ~FrameWriteBuffer() {
        if (vbuf)    { vbuf->EndAccess(bmdBufferAccessWrite);    vbuf->Release(); }
        if (vbuf153) { vbuf153->EndAccess(bmdBufferAccessWrite); vbuf153->Release(); }
        if (old)     { old->Release(); }
    }

    FrameWriteBuffer(const FrameWriteBuffer &) = delete;
    FrameWriteBuffer &operator=(const FrameWriteBuffer &) = delete;
};

static DkOutputInterface queryOutput(IDeckLink *device)
{
    DkOutputInterface iface;
    IDeckLinkOutput *out16 = nullptr;
    if (device->QueryInterface(IID_IDeckLinkOutput,
                                reinterpret_cast<void **>(&out16)) == S_OK) {
        iface.v16 = out16;
        return iface;
    }
    void *p = nullptr;
    if (device->QueryInterface(IID_IDeckLinkOutput_v15_3_1, &p) == S_OK) {
        iface.v16 = reinterpret_cast<IDeckLinkOutput *>(p);
        return iface;
    }
    IDeckLinkOutput_v14_2_1 *out14 = nullptr;
    if (device->QueryInterface(IID_IDeckLinkOutput_v14_2_1,
                                reinterpret_cast<void **>(&out14)) == S_OK)
        iface.v14 = out14;
    return iface;
}

static IDeckLinkConfiguration *queryConfig(IDeckLink *device)
{
    IDeckLinkConfiguration *cfg = nullptr;
    if (device->QueryInterface(IID_IDeckLinkConfiguration,
                                reinterpret_cast<void **>(&cfg)) == S_OK)
        return cfg;
    void *p = nullptr;
    if (device->QueryInterface(IID_IDeckLinkConfiguration_v15_3_1, &p) == S_OK)
        return reinterpret_cast<IDeckLinkConfiguration *>(p);
    if (device->QueryInterface(IID_IDeckLinkConfiguration_v10_11, &p) == S_OK)
        return reinterpret_cast<IDeckLinkConfiguration *>(p);
    if (device->QueryInterface(IID_IDeckLinkConfiguration_v10_9, &p) == S_OK)
        return reinterpret_cast<IDeckLinkConfiguration *>(p);
    if (device->QueryInterface(IID_IDeckLinkConfiguration_v10_4, &p) == S_OK)
        return reinterpret_cast<IDeckLinkConfiguration *>(p);
    return nullptr;
}

static IDeckLinkProfileAttributes *queryAttributes(IDeckLink *device)
{
    IDeckLinkProfileAttributes *attrs = nullptr;
    if (device->QueryInterface(IID_IDeckLinkProfileAttributes,
                                reinterpret_cast<void **>(&attrs)) == S_OK)
        return attrs;
    void *p = nullptr;
    if (device->QueryInterface(IID_IDeckLinkProfileAttributes_v15_3_1, &p) == S_OK)
        return reinterpret_cast<IDeckLinkProfileAttributes *>(p);
    // IDeckLinkAttributes is the pre-15.x name for the same interface (identical vtable)
    if (device->QueryInterface(IID_IDeckLinkAttributes_v10_11, &p) == S_OK)
        return reinterpret_cast<IDeckLinkProfileAttributes *>(p);
    return nullptr;
}

IDeckLink *DkDeckLinkOutput::getDevice(int index)
{
    IDeckLinkIterator *it = CreateDeckLinkIteratorInstance();
    if (!it)
        return nullptr;

    IDeckLink *device = nullptr;
    int i = 0;
    while (it->Next(&device) == S_OK) {
        if (i == index) {
            it->Release();
            return device; // caller must Release()
        }
        device->Release();
        device = nullptr;
        ++i;
    }
    it->Release();
    return nullptr;
}

// -----------------------------------------------------------------------
// Static enumeration
// -----------------------------------------------------------------------

QStringList DkDeckLinkOutput::enumerateDevices()
{
    IDeckLinkIterator *it = CreateDeckLinkIteratorInstance();
    if (!it)
        return {};

    QStringList names;
    IDeckLink *device = nullptr;
    while (it->Next(&device) == S_OK) {
        BMDString name = nullptr;
        if (device->GetDisplayName(&name) == S_OK && name) {
            names << bmdStringToQString(name);
            bmdStringFree(name);
        } else {
            names << QStringLiteral("Unknown device");
        }
        device->Release();
    }
    it->Release();
    return names;
}

QVector<DkDisplayModeInfo> DkDeckLinkOutput::enumerateModes(int deviceIndex)
{
    QVector<DkDisplayModeInfo> modes;

    IDeckLink *device = getDevice(deviceIndex);
    if (!device)
        return modes;

    DkOutputInterface output = queryOutput(device);
    if (!output.valid()) {
        device->Release();
        return modes;
    }

    IDeckLinkDisplayModeIterator *modeIt = nullptr;
    if (output.GetDisplayModeIterator(&modeIt) == S_OK) {
        IDeckLinkDisplayMode *modeObj = nullptr;
        while (modeIt->Next(&modeObj) == S_OK) {
            // Accept the mode if the device supports at least one of our formats
            bool accepted = false;
            for (const auto &fmt : kAllFormats) {
                BMDDisplayMode actualMode = bmdModeUnknown;
                BMDBool supported = 0;
                output.DoesSupportVideoMode(bmdVideoConnectionUnspecified,
                                            modeObj->GetDisplayMode(),
                                            fmt.format,
                                            bmdNoVideoOutputConversion,
                                            bmdSupportedVideoModeDefault,
                                            &actualMode, &supported);
                if (supported) {
                    accepted = true;
                    break;
                }
            }

            if (accepted) {
                DkDisplayModeInfo info;
                info.mode = modeObj->GetDisplayMode();
                info.width = modeObj->GetWidth();
                info.height = modeObj->GetHeight();
                modeObj->GetFrameRate(&info.frameDuration, &info.timeScale);
                BMDString mname = nullptr;
                if (modeObj->GetName(&mname) == S_OK && mname) {
                    info.name = bmdStringToQString(mname);
                    bmdStringFree(mname);
                }
                modes << info;
            }
            modeObj->Release();
        }
        modeIt->Release();
    }

    output.release();
    device->Release();
    return modes;
}

QVector<DkPixelFormatInfo> DkDeckLinkOutput::enumerateFormats(int deviceIndex, BMDDisplayMode displayMode)
{
    QVector<DkPixelFormatInfo> result;

    IDeckLink *device = getDevice(deviceIndex);
    if (!device)
        return result;

    DkOutputInterface output = queryOutput(device);
    if (!output.valid()) {
        device->Release();
        return result;
    }

    for (const auto &fmt : kAllFormats) {
        BMDDisplayMode actualMode = bmdModeUnknown;
        BMDBool supported = 0;
        output.DoesSupportVideoMode(bmdVideoConnectionUnspecified,
                                    displayMode,
                                    fmt.format,
                                    bmdNoVideoOutputConversion,
                                    bmdSupportedVideoModeDefault,
                                    &actualMode, &supported);
        if (supported)
            result << fmt;
    }

    output.release();
    device->Release();
    return result;
}

bool DkDeckLinkOutput::supportsDualLink(int deviceIndex)
{
    IDeckLink *device = getDevice(deviceIndex);
    if (!device)
        return false;

    IDeckLinkProfileAttributes *attrs = queryAttributes(device);
    BMDBool dual = 0;
    if (attrs) {
        attrs->GetFlag(BMDDeckLinkSupportsDualLinkSDI, &dual);
        attrs->Release();
    }
    device->Release();
    return dual;
}

// -----------------------------------------------------------------------
// Constructor / Destructor
// -----------------------------------------------------------------------

DkDeckLinkOutput::DkDeckLinkOutput(QObject *parent)
    : QObject(parent)
{
    bmdPlatformInit();
}

DkDeckLinkOutput::~DkDeckLinkOutput()
{
    stopOutput();
    bmdPlatformUninit();
}

// -----------------------------------------------------------------------
// startOutput
// -----------------------------------------------------------------------

bool DkDeckLinkOutput::startOutput(const DkOutputConfig &cfg)
{
    stopOutput();
    mLastError.clear();
    mConfig = cfg;

    mDeckLink = getDevice(cfg.deviceIndex);
    if (!mDeckLink) {
        mLastError = QStringLiteral("Device not found");
        return false;
    }

    // Apply output configuration flags
    IDeckLinkConfiguration *dlCfg = queryConfig(mDeckLink);
    if (dlCfg) {
        dlCfg->SetInt(bmdDeckLinkConfigSDIOutputLinkConfiguration,
                      static_cast<int64_t>(cfg.linkConfig));
        dlCfg->SetFlag(bmdDeckLinkConfigRec2020Output,      cfg.rec2020);
        dlCfg->SetFlag(bmdDeckLinkConfigSMPTELevelAOutput,  cfg.smpteLevelA);
        dlCfg->SetFlag(bmdDeckLinkConfigOutput1080pAsPsF,   cfg.outputAsPsF);
        dlCfg->Release();
    }

    mDeckLinkOutput = queryOutput(mDeckLink);
    if (!mDeckLinkOutput.valid()) {
        mLastError = QStringLiteral("Device does not support video output");
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    if (mDeckLinkOutput.EnableVideoOutput(cfg.mode.mode, bmdVideoOutputFlagDefault) != S_OK) {
        mLastError = QStringLiteral("EnableVideoOutput failed");
        mDeckLinkOutput.release();
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    if (mDeckLinkOutput.SetScheduledFrameCompletionCallback(this) != S_OK) {
        mLastError = QStringLiteral("SetScheduledFrameCompletionCallback failed");
        mDeckLinkOutput.DisableVideoOutput();
        mDeckLinkOutput.release();
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    const long rowBytes = computeRowBytes(cfg.mode.width, cfg.pixelFormat);
    if (mDeckLinkOutput.CreateVideoFrame(static_cast<int32_t>(cfg.mode.width),
                                          static_cast<int32_t>(cfg.mode.height),
                                          static_cast<int32_t>(rowBytes),
                                          cfg.pixelFormat,
                                          bmdFrameFlagDefault,
                                          &mFrame) != S_OK || !mFrame) {
        mLastError = QStringLiteral("CreateVideoFrame failed");
        mDeckLinkOutput.DisableVideoOutput();
        mDeckLinkOutput.release();
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    // Fill with black
    {
        FrameWriteBuffer wb(mFrame);
        if (wb.buf)
            std::memset(wb.buf, 0,
                        static_cast<size_t>(mFrame->GetRowBytes() * mFrame->GetHeight()));
    }

    mRunning = true;
    mNextFrameTime = 0;

    // Pre-schedule 3 frames to prime the pipeline.
    // Do NOT AddRef here — the SDK AddRefs when scheduling and Releases when completed.
    for (int i = 0; i < 3; ++i) {
        mDeckLinkOutput.ScheduleVideoFrame(mFrame, mNextFrameTime,
                                            cfg.mode.frameDuration,
                                            cfg.mode.timeScale);
        mNextFrameTime += cfg.mode.frameDuration;
    }

    mDeckLinkOutput.StartScheduledPlayback(0, cfg.mode.timeScale, 1.0);
    return true;
}

// -----------------------------------------------------------------------
// sendImage — called from Qt main thread
// -----------------------------------------------------------------------

void DkDeckLinkOutput::sendImage(const QImage &img)
{
    if (!mRunning)
        return;
    QMutexLocker lk(&mPendingMutex);
    mPendingImage = img.convertToFormat(QImage::Format_ARGB32);
    mHasPending = true;
}

// -----------------------------------------------------------------------
// stopOutput
// -----------------------------------------------------------------------

void DkDeckLinkOutput::stopOutput()
{
    if (!mRunning)
        return;
    mRunning = false;

    if (mDeckLinkOutput.valid()) {
        mDeckLinkOutput.StopScheduledPlayback(0, nullptr, mConfig.mode.timeScale);
        mDeckLinkOutput.SetScheduledFrameCompletionCallback(nullptr);
        mDeckLinkOutput.DisableVideoOutput();
        mDeckLinkOutput.release();
    }
    if (mFrame) {
        mFrame->Release();
        mFrame = nullptr;
    }
    if (mDeckLink) {
        mDeckLink->Release();
        mDeckLink = nullptr;
    }

    QMutexLocker lk(&mPendingMutex);
    mPendingImage = QImage();
    mHasPending = false;
}

// -----------------------------------------------------------------------
// ScheduledFrameCompleted — DeckLink driver thread
// -----------------------------------------------------------------------

HRESULT DkDeckLinkOutput::ScheduledFrameCompleted(IDeckLinkVideoFrame *completedFrame,
                                                    BMDOutputFrameCompletionResult)
{
    if (!mRunning || !mDeckLinkOutput.valid())
        return S_OK;

    QImage pending;
    bool hasPending = false;
    {
        QMutexLocker lk(&mPendingMutex);
        if (mHasPending) {
            pending = std::move(mPendingImage);
            mPendingImage = QImage();
            mHasPending = false;
            hasPending = true;
        }
    }

    // We only ever schedule one frame (mFrame); completedFrame IS mFrame.
    // Avoid a QueryInterface whose IID may have changed across SDK versions.
    if (hasPending && mFrame)
        fillFrame(mFrame, pending);

    // Re-schedule the same frame to hold the still image (SDK handles its own ref count)
    if (mFrame)
        mDeckLinkOutput.ScheduleVideoFrame(mFrame, mNextFrameTime,
                                            mConfig.mode.frameDuration,
                                            mConfig.mode.timeScale);
    mNextFrameTime += mConfig.mode.frameDuration;
    return S_OK;
}

HRESULT DkDeckLinkOutput::ScheduledPlaybackHasStopped()
{
    return S_OK;
}

// -----------------------------------------------------------------------
// fillFrame — scale source and dispatch to format writer
// -----------------------------------------------------------------------

void DkDeckLinkOutput::fillFrame(IDeckLinkMutableVideoFrame *frame, const QImage &src)
{
    FrameWriteBuffer wb(frame);
    if (!wb.buf)
        return;

    void *dst = wb.buf;
    const long rowBytes = frame->GetRowBytes();
    const int fw = static_cast<int>(frame->GetWidth());
    const int fh = static_cast<int>(frame->GetHeight());

    // Letterbox/pillarbox onto a black ARGB32 canvas
    QImage canvas;
    if (src.width() != fw || src.height() != fh) {
        QImage scaled = src.scaled(fw, fh, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        canvas = QImage(fw, fh, QImage::Format_ARGB32);
        canvas.fill(Qt::black);
        const int ox = (fw - scaled.width()) / 2;
        const int oy = (fh - scaled.height()) / 2;
        for (int y = 0; y < scaled.height(); ++y)
            std::memcpy(canvas.scanLine(oy + y) + ox * 4,
                        scaled.constScanLine(y),
                        static_cast<size_t>(scaled.width() * 4));
    } else {
        canvas = src; // already correct size and format
    }

    switch (mConfig.pixelFormat) {
    case bmdFormat8BitBGRA:   fillBGRA8  (dst, rowBytes, canvas); break;
    case bmdFormat10BitRGB:   fillRGB10  (dst, rowBytes, canvas); break;
    case bmdFormat12BitRGB:   fillRGB12  (dst, rowBytes, canvas); break;
    case bmdFormat12BitRGBLE: fillRGB12LE(dst, rowBytes, canvas); break;
    case bmdFormat8BitYUV:    fillYUV8   (dst, rowBytes, canvas); break;
    case bmdFormat10BitYUV:   fillYUV10  (dst, rowBytes, canvas); break;
    default: break;
    }
    // FrameWriteBuffer destructor calls EndAccess + Release
}

// -----------------------------------------------------------------------
// fillBGRA8 — bmdFormat8BitBGRA
//   Memory layout per pixel: [B, G, R, A]
//   Legal range: scale each channel 0-255 → 16-235 (SMPTE 8-bit RGB).
// -----------------------------------------------------------------------

void DkDeckLinkOutput::fillBGRA8(void *dst, long rowBytes, const QImage &src) const
{
    const int w = src.width();
    const int h = src.height();
    const bool legal = mConfig.legalRange;

    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        uint8_t *dstRow = static_cast<uint8_t *>(dst) + y * rowBytes;
        for (int x = 0; x < w; ++x) {
            const QRgb px = srcRow[x];
            uint8_t r = static_cast<uint8_t>(qRed(px));
            uint8_t g = static_cast<uint8_t>(qGreen(px));
            uint8_t b = static_cast<uint8_t>(qBlue(px));
            if (legal) {
                r = static_cast<uint8_t>(16 + (r * 219 + 127) / 255);
                g = static_cast<uint8_t>(16 + (g * 219 + 127) / 255);
                b = static_cast<uint8_t>(16 + (b * 219 + 127) / 255);
            }
            dstRow[x * 4 + 0] = b;
            dstRow[x * 4 + 1] = g;
            dstRow[x * 4 + 2] = r;
            dstRow[x * 4 + 3] = 0xFF;
        }
    }
}

// -----------------------------------------------------------------------
// fillRGB10 — bmdFormat10BitRGB  (r210, big-endian)
//
// SDK spec: "Packed as 2:10:10:10" — the 2 padding bits are at the MSB.
// 32-bit big-endian word layout:
//   bits [31:30] = 00 (padding)
//   bits [29:20] = R[9:0]
//   bits [19:10] = G[9:0]
//   bits  [9: 0] = B[9:0]
//
// Note: the SDK marks this format as SMPTE legal levels (64-940).
// Selecting "full" sends 0-1023 which is outside the defined range;
// some displays may clip, but extended-range workflows need it.
// -----------------------------------------------------------------------

void DkDeckLinkOutput::fillRGB10(void *dst, long rowBytes, const QImage &src) const
{
    const int w = src.width();
    const int h = src.height();
    const bool legal = mConfig.legalRange;

    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        uint8_t *dstRow = static_cast<uint8_t *>(dst) + y * rowBytes;

        for (int x = 0; x < w; ++x) {
            const QRgb px = srcRow[x];
            const int r8 = qRed(px), g8 = qGreen(px), b8 = qBlue(px);

            uint32_t r10, g10, b10;
            if (legal) {
                r10 = static_cast<uint32_t>(64 + (r8 * 876 + 127) / 255);
                g10 = static_cast<uint32_t>(64 + (g8 * 876 + 127) / 255);
                b10 = static_cast<uint32_t>(64 + (b8 * 876 + 127) / 255);
            } else {
                // Expand 8→10 bit: duplicate top 2 bits into the bottom to reach 1023 for 255.
                r10 = (static_cast<uint32_t>(r8) << 2) | (static_cast<uint32_t>(r8) >> 6);
                g10 = (static_cast<uint32_t>(g8) << 2) | (static_cast<uint32_t>(g8) >> 6);
                b10 = (static_cast<uint32_t>(b8) << 2) | (static_cast<uint32_t>(b8) >> 6);
            }

            // Pack: 00[R9:0][G9:0][B9:0] in a 32-bit big-endian word
            const uint32_t word = (r10 << 20) | (g10 << 10) | b10;
            uint8_t *p = dstRow + x * 4;
            p[0] = static_cast<uint8_t>((word >> 24) & 0xFF);
            p[1] = static_cast<uint8_t>((word >> 16) & 0xFF);
            p[2] = static_cast<uint8_t>((word >>  8) & 0xFF);
            p[3] = static_cast<uint8_t>((word      ) & 0xFF);
        }
    }
}

// -----------------------------------------------------------------------
// fillRGB12 / fillRGB12LE — bmdFormat12BitRGB (R12B) / bmdFormat12BitRGBLE (R12L)
//
// Both formats pack 2 pixels (R0,G0,B0, R1,G1,B1) into 9 bytes (72 bits).
//
// R12B — big-endian, R0 in the most-significant bits:
//   byte 0: R0[11:4]
//   byte 1: R0[3:0]  G0[11:8]
//   byte 2: G0[7:0]
//   byte 3: B0[11:4]
//   byte 4: B0[3:0]  R1[11:8]
//   byte 5: R1[7:0]
//   byte 6: G1[11:4]
//   byte 7: G1[3:0]  B1[11:8]
//   byte 8: B1[7:0]
//
// R12L — little-endian (B1 in the least-significant bits):
//   byte 0: B1[7:0]
//   byte 1: G1[3:0]  B1[11:8]
//   byte 2: G1[11:4]
//   byte 3: R1[7:0]
//   byte 4: B0[3:0]  R1[11:8]
//   byte 5: B0[11:4]
//   byte 6: G0[7:0]
//   byte 7: R0[3:0]  G0[11:8]
//   byte 8: R0[11:4]
//
// Legal range (SMPTE 12-bit): 256–3760 (= 64–940 scaled ×4).
// Full range: expand 8→12 bit by replicating the top nibble into the bottom.
// -----------------------------------------------------------------------

static inline uint32_t expand8to12(int v8, bool legal)
{
    if (legal)
        return static_cast<uint32_t>(256 + (v8 * 3504 + 127) / 255);
    return static_cast<uint32_t>((v8 << 4) | (v8 >> 4));
}

void DkDeckLinkOutput::fillRGB12(void *dst, long rowBytes, const QImage &src) const
{
    const int w = src.width();
    const int h = src.height();
    const bool legal = mConfig.legalRange;

    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        uint8_t *dstRow = static_cast<uint8_t *>(dst) + y * rowBytes;

        for (int x = 0; x < w; x += 2) {
            const QRgb px0 = srcRow[x];
            const QRgb px1 = (x + 1 < w) ? srcRow[x + 1] : px0;

            const uint32_t R0 = expand8to12(qRed(px0),   legal);
            const uint32_t G0 = expand8to12(qGreen(px0), legal);
            const uint32_t B0 = expand8to12(qBlue(px0),  legal);
            const uint32_t R1 = expand8to12(qRed(px1),   legal);
            const uint32_t G1 = expand8to12(qGreen(px1), legal);
            const uint32_t B1 = expand8to12(qBlue(px1),  legal);

            uint8_t *p = dstRow + (x / 2) * 9;
            p[0] = static_cast<uint8_t>(R0 >> 4);
            p[1] = static_cast<uint8_t>((R0 & 0xF) << 4 | G0 >> 8);
            p[2] = static_cast<uint8_t>(G0 & 0xFF);
            p[3] = static_cast<uint8_t>(B0 >> 4);
            p[4] = static_cast<uint8_t>((B0 & 0xF) << 4 | R1 >> 8);
            p[5] = static_cast<uint8_t>(R1 & 0xFF);
            p[6] = static_cast<uint8_t>(G1 >> 4);
            p[7] = static_cast<uint8_t>((G1 & 0xF) << 4 | B1 >> 8);
            p[8] = static_cast<uint8_t>(B1 & 0xFF);
        }
    }
}

void DkDeckLinkOutput::fillRGB12LE(void *dst, long rowBytes, const QImage &src) const
{
    const int w = src.width();
    const int h = src.height();
    const bool legal = mConfig.legalRange;

    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        uint8_t *dstRow = static_cast<uint8_t *>(dst) + y * rowBytes;

        for (int x = 0; x < w; x += 2) {
            const QRgb px0 = srcRow[x];
            const QRgb px1 = (x + 1 < w) ? srcRow[x + 1] : px0;

            const uint32_t R0 = expand8to12(qRed(px0),   legal);
            const uint32_t G0 = expand8to12(qGreen(px0), legal);
            const uint32_t B0 = expand8to12(qBlue(px0),  legal);
            const uint32_t R1 = expand8to12(qRed(px1),   legal);
            const uint32_t G1 = expand8to12(qGreen(px1), legal);
            const uint32_t B1 = expand8to12(qBlue(px1),  legal);

            uint8_t *p = dstRow + (x / 2) * 9;
            p[0] = static_cast<uint8_t>(B1 & 0xFF);
            p[1] = static_cast<uint8_t>((G1 & 0xF) << 4 | B1 >> 8);
            p[2] = static_cast<uint8_t>(G1 >> 4);
            p[3] = static_cast<uint8_t>(R1 & 0xFF);
            p[4] = static_cast<uint8_t>((B0 & 0xF) << 4 | R1 >> 8);
            p[5] = static_cast<uint8_t>(B0 >> 4);
            p[6] = static_cast<uint8_t>(G0 & 0xFF);
            p[7] = static_cast<uint8_t>((R0 & 0xF) << 4 | G0 >> 8);
            p[8] = static_cast<uint8_t>(R0 >> 4);
        }
    }
}

// -----------------------------------------------------------------------
// Rec.709 RGB→YCbCr helpers
// -----------------------------------------------------------------------

static void rgbToYCbCr8(int r, int g, int b, bool legal,
                         uint8_t &Y, uint8_t &Cb, uint8_t &Cr)
{
    // Rec.709 matrix ×65536 (integer approximation)
    int yi  = ( 13933 * r + 46871 * g +  4732 * b) >> 16;
    int cbi = ((-7471 * r - 25145 * g + 32616 * b) >> 16) + 128;
    int cri = ((32616 * r - 27145 * g -  5471 * b) >> 16) + 128;

    yi  = std::clamp(yi,  0, 255);
    cbi = std::clamp(cbi, 0, 255);
    cri = std::clamp(cri, 0, 255);

    if (legal) {
        Y  = static_cast<uint8_t>(16  + (yi  * 219 + 127) / 255);
        Cb = static_cast<uint8_t>(16  + (cbi * 224 + 127) / 255);
        Cr = static_cast<uint8_t>(16  + (cri * 224 + 127) / 255);
    } else {
        Y  = static_cast<uint8_t>(yi);
        Cb = static_cast<uint8_t>(cbi);
        Cr = static_cast<uint8_t>(cri);
    }
}

static void rgbToYCbCr10(int r, int g, int b, bool legal,
                          uint16_t &Y, uint16_t &Cb, uint16_t &Cr)
{
    int yi  = ( 13933 * r + 46871 * g +  4732 * b) >> 16;
    int cbi = ((-7471 * r - 25145 * g + 32616 * b) >> 16) + 128;
    int cri = ((32616 * r - 27145 * g -  5471 * b) >> 16) + 128;

    yi  = std::clamp(yi,  0, 255);
    cbi = std::clamp(cbi, 0, 255);
    cri = std::clamp(cri, 0, 255);

    if (legal) {
        Y  = static_cast<uint16_t>(64  + (yi  * 876 + 127) / 255);
        Cb = static_cast<uint16_t>(64  + (cbi * 896 + 127) / 255);
        Cr = static_cast<uint16_t>(64  + (cri * 896 + 127) / 255);
    } else {
        Y  = static_cast<uint16_t>(yi  << 2);
        Cb = static_cast<uint16_t>(cbi << 2);
        Cr = static_cast<uint16_t>(cri << 2);
    }
}

// -----------------------------------------------------------------------
// fillYUV8 — bmdFormat8BitYUV  (2vuy, 4:2:2)
//   Per 2 pixels: [Cb, Y0, Cr, Y1]
// -----------------------------------------------------------------------

void DkDeckLinkOutput::fillYUV8(void *dst, long rowBytes, const QImage &src) const
{
    const int w = src.width();
    const int h = src.height();
    const bool legal = mConfig.legalRange;

    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        uint8_t *dstRow = static_cast<uint8_t *>(dst) + y * rowBytes;

        for (int x = 0; x < w; x += 2) {
            const QRgb px0 = srcRow[x];
            const QRgb px1 = (x + 1 < w) ? srcRow[x + 1] : px0;

            uint8_t Y0, Cb0, Cr0, Y1, Cb1, Cr1;
            rgbToYCbCr8(qRed(px0), qGreen(px0), qBlue(px0), legal, Y0, Cb0, Cr0);
            rgbToYCbCr8(qRed(px1), qGreen(px1), qBlue(px1), legal, Y1, Cb1, Cr1);

            const uint8_t Cb = static_cast<uint8_t>((Cb0 + Cb1) / 2);
            const uint8_t Cr = static_cast<uint8_t>((Cr0 + Cr1) / 2);

            const int out = x * 2;
            dstRow[out + 0] = Cb;
            dstRow[out + 1] = Y0;
            dstRow[out + 2] = Cr;
            dstRow[out + 3] = Y1;
        }
    }
}

// -----------------------------------------------------------------------
// fillYUV10 — bmdFormat10BitYUV  (v210, 4:2:2)
//
// v210 packs 6 pixels (6Y + 3Cb + 3Cr = 12 × 10-bit) into 4 × 32-bit words.
// Each 32-bit LE word holds 3 × 10-bit samples in bits [9:0], [19:10], [29:20].
//
//   Word 0:  Cb0 | Y0<<10 | Cr0<<20
//   Word 1:  Y1  | Cb1<<10 | Y2<<20
//   Word 2:  Cr1 | Y3<<10  | Cb2<<20
//   Word 3:  Y4  | Cr2<<10 | Y5<<20
// -----------------------------------------------------------------------

void DkDeckLinkOutput::fillYUV10(void *dst, long rowBytes, const QImage &src) const
{
    const int w = src.width();
    const int h = src.height();
    const bool legal = mConfig.legalRange;

    const uint16_t blackY  = legal ? 64  : 0;
    const uint16_t blackCb = legal ? 512 : 512;
    const uint16_t blackCr = legal ? 512 : 512;

    for (int y = 0; y < h; ++y) {
        const QRgb *srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        uint32_t *dstRow = reinterpret_cast<uint32_t *>(
            static_cast<uint8_t *>(dst) + y * rowBytes);

        const int numGroups = (w + 5) / 6;
        for (int g = 0; g < numGroups; ++g) {
            const int base = g * 6;

            uint16_t Y[6]  = { blackY,  blackY,  blackY,  blackY,  blackY,  blackY  };
            uint16_t Cb[3] = { blackCb, blackCb, blackCb };
            uint16_t Cr[3] = { blackCr, blackCr, blackCr };

            for (int k = 0; k < 6; ++k) {
                if (base + k >= w)
                    break;
                const QRgb px = srcRow[base + k];
                uint16_t tY, tCb, tCr;
                rgbToYCbCr10(qRed(px), qGreen(px), qBlue(px), legal, tY, tCb, tCr);
                Y[k] = tY;
                // Cosited chroma: sample at even positions, average with odd neighbor
                if (k % 2 == 0) {
                    Cb[k / 2] = tCb;
                    Cr[k / 2] = tCr;
                } else {
                    Cb[k / 2] = static_cast<uint16_t>((Cb[k / 2] + tCb) / 2);
                    Cr[k / 2] = static_cast<uint16_t>((Cr[k / 2] + tCr) / 2);
                }
            }

            uint32_t *w4 = dstRow + g * 4;
            w4[0] = (uint32_t(Cb[0]) & 0x3FF)
                  | ((uint32_t(Y[0])  & 0x3FF) << 10)
                  | ((uint32_t(Cr[0]) & 0x3FF) << 20);
            w4[1] = (uint32_t(Y[1])  & 0x3FF)
                  | ((uint32_t(Cb[1]) & 0x3FF) << 10)
                  | ((uint32_t(Y[2])  & 0x3FF) << 20);
            w4[2] = (uint32_t(Cr[1]) & 0x3FF)
                  | ((uint32_t(Y[3])  & 0x3FF) << 10)
                  | ((uint32_t(Cb[2]) & 0x3FF) << 20);
            w4[3] = (uint32_t(Y[4])  & 0x3FF)
                  | ((uint32_t(Cr[2]) & 0x3FF) << 10)
                  | ((uint32_t(Y[5])  & 0x3FF) << 20);
        }
    }
}

// -----------------------------------------------------------------------
// IUnknown
// -----------------------------------------------------------------------

HRESULT DkDeckLinkOutput::QueryInterface(REFIID iid, LPVOID *ppv)
{
    if (!ppv)
        return E_POINTER;
    if (refiidEqual(iid, IID_IDeckLinkVideoOutputCallback)) {
        *ppv = static_cast<IDeckLinkVideoOutputCallback *>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

ULONG DkDeckLinkOutput::AddRef()
{
    return ++mRefCount;
}

ULONG DkDeckLinkOutput::Release()
{
    const ULONG ref = --mRefCount;
    if (ref == 0)
        delete this;
    return ref;
}

} // namespace nmc
