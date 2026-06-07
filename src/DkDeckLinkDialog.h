#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>

#include "DkDeckLinkOutput.h"

namespace nmc
{

class DkDeckLinkDialog : public QDialog
{
    Q_OBJECT

public:
    // Pass the current output so the dialog can pre-select the last config.
    explicit DkDeckLinkDialog(DkDeckLinkOutput *output, QWidget *parent = nullptr);

    DkOutputConfig config() const;

    // True if the user changed any setting relative to what was running before.
    bool configChanged() const { return mConfigChanged; }

private slots:
    void onDeviceChanged(int index);
    void onModeChanged(int index);
    void onFormatChanged(int index);

private:
    void populateDevices(int selectIndex);
    void populateModes(int deviceIndex, BMDDisplayMode selectMode);
    void populateFormats(int deviceIndex, BMDDisplayMode mode, BMDPixelFormat selectFormat);
    void updateLinkVisibility(int deviceIndex);
    void updateLevelsVisibility();

    QComboBox *mDeviceCombo      = nullptr;
    QComboBox *mModeCombo        = nullptr;
    QComboBox *mFormatCombo      = nullptr;
    QComboBox *mLinkCombo        = nullptr;
    QCheckBox *mLegalCheck       = nullptr;
    QCheckBox *mReleaseCheck     = nullptr;
    QLabel    *mStatusLabel      = nullptr;
    QLabel    *mLinkLabel        = nullptr;

    QVector<DkDisplayModeInfo> mModes;
    QVector<DkPixelFormatInfo> mFormats;

    DkDeckLinkOutput *mOutput = nullptr; // non-owning, only for configChanged() comparison

    DkOutputConfig mOriginalConfig;
    bool mHasOriginal = false;       // true when mOriginalConfig came from a running output
    bool mConfigChanged = false;

    // When pre-populating from QSettings (not from a running output) we match
    // the device by display name rather than by index, since index can shift.
    QString mSavedDeviceName;
};

} // namespace nmc
