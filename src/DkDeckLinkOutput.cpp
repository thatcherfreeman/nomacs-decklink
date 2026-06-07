#include "DkDeckLinkOutput.h"
#include "DkDeckLinkPlatform.h"

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
    case bmdFormat8BitBGRA:  return width * 4;
    case bmdFormat10BitRGB:  return width * 4; // 1 32-bit word per pixel
    case bmdFormat8BitYUV:   return ((width + 1) / 2) * 4; // Cb+Y0+Cr+Y1 per pair
    case bmdFormat10BitYUV:  return ((width + 47) / 48) * 128; // v210: ceil(w/48)*128 bytes
    default:                 return width * 4;
    }
}

static const QVector<DkPixelFormatInfo> kAllFormats = {
    { bmdFormat8BitBGRA, QStringLiteral("8-bit RGBA")             },
    { bmdFormat10BitRGB, QStringLiteral("10-bit RGB (r210)")       },
    { bmdFormat8BitYUV,  QStringLiteral("8-bit YUV 4:2:2 (2vuy)") },
    { bmdFormat10BitYUV, QStringLiteral("10-bit YUV 4:2:2 (v210)")},
};

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

    IDeckLinkOutput *output = nullptr;
    if (device->QueryInterface(IID_IDeckLinkOutput, reinterpret_cast<void **>(&output)) != S_OK) {
        device->Release();
        return modes;
    }

    IDeckLinkDisplayModeIterator *modeIt = nullptr;
    if (output->GetDisplayModeIterator(&modeIt) == S_OK) {
        IDeckLinkDisplayMode *modeObj = nullptr;
        while (modeIt->Next(&modeObj) == S_OK) {
            // Accept the mode if the device supports at least one of our formats
            bool accepted = false;
            for (const auto &fmt : kAllFormats) {
                BMDDisplayMode actualMode = bmdModeUnknown;
                BMDBool supported = 0;
                output->DoesSupportVideoMode(bmdVideoConnectionUnspecified,
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

    output->Release();
    device->Release();
    return modes;
}

QVector<DkPixelFormatInfo> DkDeckLinkOutput::enumerateFormats(int deviceIndex, BMDDisplayMode displayMode)
{
    QVector<DkPixelFormatInfo> result;

    IDeckLink *device = getDevice(deviceIndex);
    if (!device)
        return result;

    IDeckLinkOutput *output = nullptr;
    if (device->QueryInterface(IID_IDeckLinkOutput, reinterpret_cast<void **>(&output)) != S_OK) {
        device->Release();
        return result;
    }

    for (const auto &fmt : kAllFormats) {
        BMDDisplayMode actualMode = bmdModeUnknown;
        BMDBool supported = 0;
        output->DoesSupportVideoMode(bmdVideoConnectionUnspecified,
                                     displayMode,
                                     fmt.format,
                                     bmdNoVideoOutputConversion,
                                     bmdSupportedVideoModeDefault,
                                     &actualMode, &supported);
        if (supported)
            result << fmt;
    }

    output->Release();
    device->Release();
    return result;
}

bool DkDeckLinkOutput::supportsDualLink(int deviceIndex)
{
    IDeckLink *device = getDevice(deviceIndex);
    if (!device)
        return false;

    IDeckLinkProfileAttributes *attrs = nullptr;
    BMDBool dual = 0;
    if (device->QueryInterface(IID_IDeckLinkProfileAttributes,
                                reinterpret_cast<void **>(&attrs)) == S_OK) {
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

    // Configure SDI link mode
    IDeckLinkConfiguration *dlCfg = nullptr;
    if (mDeckLink->QueryInterface(IID_IDeckLinkConfiguration,
                                   reinterpret_cast<void **>(&dlCfg)) == S_OK) {
        dlCfg->SetInt(bmdDeckLinkConfigSDIOutputLinkConfiguration,
                      static_cast<int64_t>(cfg.linkConfig));
        dlCfg->Release();
    }

    if (mDeckLink->QueryInterface(IID_IDeckLinkOutput,
                                   reinterpret_cast<void **>(&mDeckLinkOutput)) != S_OK) {
        mLastError = QStringLiteral("Device does not support video output");
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    if (mDeckLinkOutput->EnableVideoOutput(cfg.mode.mode, bmdVideoOutputFlagDefault) != S_OK) {
        mLastError = QStringLiteral("EnableVideoOutput failed");
        mDeckLinkOutput->Release();
        mDeckLinkOutput = nullptr;
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    if (mDeckLinkOutput->SetScheduledFrameCompletionCallback(this) != S_OK) {
        mLastError = QStringLiteral("SetScheduledFrameCompletionCallback failed");
        mDeckLinkOutput->DisableVideoOutput();
        mDeckLinkOutput->Release();
        mDeckLinkOutput = nullptr;
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    const long rowBytes = computeRowBytes(cfg.mode.width, cfg.pixelFormat);
    if (mDeckLinkOutput->CreateVideoFrame(static_cast<int32_t>(cfg.mode.width),
                                           static_cast<int32_t>(cfg.mode.height),
                                           static_cast<int32_t>(rowBytes),
                                           cfg.pixelFormat,
                                           bmdFrameFlagDefault,
                                           &mFrame) != S_OK || !mFrame) {
        mLastError = QStringLiteral("CreateVideoFrame failed");
        mDeckLinkOutput->DisableVideoOutput();
        mDeckLinkOutput->Release();
        mDeckLinkOutput = nullptr;
        mDeckLink->Release();
        mDeckLink = nullptr;
        return false;
    }

    // Fill with black
    {
        IDeckLinkVideoBuffer *vbuf = nullptr;
        if (mFrame->QueryInterface(IID_IDeckLinkVideoBuffer,
                                    reinterpret_cast<void **>(&vbuf)) == S_OK && vbuf) {
            vbuf->StartAccess(bmdBufferAccessWrite);
            void *buf = nullptr;
            if (vbuf->GetBytes(&buf) == S_OK && buf)
                std::memset(buf, 0,
                            static_cast<size_t>(mFrame->GetRowBytes() * mFrame->GetHeight()));
            vbuf->EndAccess(bmdBufferAccessWrite);
            vbuf->Release();
        }
    }

    mRunning = true;
    mNextFrameTime = 0;

    // Pre-schedule 3 frames to prime the pipeline.
    // Do NOT AddRef here — the SDK AddRefs when scheduling and Releases when completed.
    for (int i = 0; i < 3; ++i) {
        mDeckLinkOutput->ScheduleVideoFrame(mFrame, mNextFrameTime,
                                             cfg.mode.frameDuration,
                                             cfg.mode.timeScale);
        mNextFrameTime += cfg.mode.frameDuration;
    }

    mDeckLinkOutput->StartScheduledPlayback(0, cfg.mode.timeScale, 1.0);
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

    if (mDeckLinkOutput) {
        mDeckLinkOutput->StopScheduledPlayback(0, nullptr, mConfig.mode.timeScale);
        mDeckLinkOutput->SetScheduledFrameCompletionCallback(nullptr);
        mDeckLinkOutput->DisableVideoOutput();
        mDeckLinkOutput->Release();
        mDeckLinkOutput = nullptr;
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
    if (!mRunning || !mDeckLinkOutput)
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

    if (hasPending && completedFrame) {
        IDeckLinkMutableVideoFrame *mf = nullptr;
        if (completedFrame->QueryInterface(IID_IDeckLinkMutableVideoFrame,
                                           reinterpret_cast<void **>(&mf)) == S_OK && mf) {
            fillFrame(mf, pending);
            mf->Release();
        }
    }

    // Re-schedule the same frame to hold the still image (SDK handles its own ref count)
    if (mFrame)
        mDeckLinkOutput->ScheduleVideoFrame(mFrame, mNextFrameTime,
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
    IDeckLinkVideoBuffer *vbuf = nullptr;
    if (frame->QueryInterface(IID_IDeckLinkVideoBuffer,
                               reinterpret_cast<void **>(&vbuf)) != S_OK || !vbuf)
        return;

    vbuf->StartAccess(bmdBufferAccessWrite);
    void *dst = nullptr;
    vbuf->GetBytes(&dst);
    if (!dst) {
        vbuf->EndAccess(bmdBufferAccessWrite);
        vbuf->Release();
        return;
    }

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
    case bmdFormat8BitBGRA: fillBGRA8(dst, rowBytes, canvas); break;
    case bmdFormat10BitRGB: fillRGB10(dst, rowBytes, canvas); break;
    case bmdFormat8BitYUV:  fillYUV8 (dst, rowBytes, canvas); break;
    case bmdFormat10BitYUV: fillYUV10(dst, rowBytes, canvas); break;
    default: break;
    }

    vbuf->EndAccess(bmdBufferAccessWrite);
    vbuf->Release();
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
