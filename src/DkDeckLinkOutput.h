#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>
#include <atomic>

#include "DeckLinkAPI.h"

namespace nmc
{

// -----------------------------------------------------------------------
// Data structures
// -----------------------------------------------------------------------

struct DkDisplayModeInfo {
    BMDDisplayMode mode = 0;
    QString name;
    long width = 0;
    long height = 0;
    BMDTimeValue frameDuration = 0;
    BMDTimeScale timeScale = 0;
};

struct DkPixelFormatInfo {
    BMDPixelFormat format;
    QString label; // shown in the dialog
};

struct DkOutputConfig {
    int deviceIndex = -1;
    DkDisplayModeInfo mode;
    BMDPixelFormat pixelFormat = bmdFormat8BitBGRA;
    bool legalRange = true;                                    // YUV only: legal (SMPTE) vs full
    BMDLinkConfiguration linkConfig = bmdLinkConfigurationSingleLink;
};

// -----------------------------------------------------------------------
// DkDeckLinkOutput
// -----------------------------------------------------------------------

class DkDeckLinkOutput : public QObject, public IDeckLinkVideoOutputCallback
{
    Q_OBJECT

public:
    explicit DkDeckLinkOutput(QObject *parent = nullptr);
    ~DkDeckLinkOutput() override;

    // Enumerate available DeckLink devices
    static QStringList enumerateDevices();

    // Enumerate output modes that the device supports for at least one of our pixel formats
    static QVector<DkDisplayModeInfo> enumerateModes(int deviceIndex);

    // Enumerate pixel formats supported by this device + mode combination
    static QVector<DkPixelFormatInfo> enumerateFormats(int deviceIndex, BMDDisplayMode mode);

    // Whether the device supports dual-link SDI output
    static bool supportsDualLink(int deviceIndex);

    // Start continuous output with the given configuration.
    bool startOutput(const DkOutputConfig &cfg);

    // Queue a new image (consumed safely inside the callback).
    void sendImage(const QImage &img);

    void stopOutput();

    bool isRunning() const { return mRunning; }
    QString lastError() const { return mLastError; }
    const DkOutputConfig &currentConfig() const { return mConfig; }

    // IDeckLinkVideoOutputCallback
    HRESULT ScheduledFrameCompleted(IDeckLinkVideoFrame *completedFrame,
                                    BMDOutputFrameCompletionResult result) override;
    HRESULT ScheduledPlaybackHasStopped() override;

    // IUnknown
    HRESULT QueryInterface(REFIID iid, LPVOID *ppv) override;
    ULONG AddRef() override;
    ULONG Release() override;

private:
    void fillFrame(IDeckLinkMutableVideoFrame *frame, const QImage &src);

    void fillBGRA8(void *dst, long rowBytes, const QImage &src) const;
    void fillRGB10(void *dst, long rowBytes, const QImage &src) const;
    void fillYUV8(void *dst, long rowBytes, const QImage &src) const;
    void fillYUV10(void *dst, long rowBytes, const QImage &src) const;

    static IDeckLink *getDevice(int index);

    IDeckLink *mDeckLink = nullptr;
    IDeckLinkOutput *mDeckLinkOutput = nullptr;
    IDeckLinkMutableVideoFrame *mFrame = nullptr;

    DkOutputConfig mConfig;
    BMDTimeValue mNextFrameTime = 0;

    QMutex mPendingMutex;
    QImage mPendingImage;
    bool mHasPending = false;

    QString mLastError;
    std::atomic<ULONG> mRefCount{1};
    bool mRunning = false;
};

} // namespace nmc
