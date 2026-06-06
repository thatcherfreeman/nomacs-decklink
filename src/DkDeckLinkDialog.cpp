#include "DkDeckLinkDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSettings>
#include <QVBoxLayout>

namespace nmc
{

DkDeckLinkDialog::DkDeckLinkDialog(DkDeckLinkOutput *output, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("DeckLink Output"));
    setMinimumWidth(420);

    mOutput = output;

    if (output && output->isRunning()) {
        mOriginalConfig = output->currentConfig();
        mHasOriginal = true;
    } else {
        // Restore last-used settings from a previous session
        QSettings s;
        s.beginGroup(QStringLiteral("DeckLinkOutput"));
        if (s.contains(QStringLiteral("deviceName"))) {
            mSavedDeviceName          = s.value(QStringLiteral("deviceName")).toString();
            mOriginalConfig.mode.mode = static_cast<BMDDisplayMode>(
                                            s.value(QStringLiteral("displayMode"), 0U).toUInt());
            mOriginalConfig.pixelFormat = static_cast<BMDPixelFormat>(
                                            s.value(QStringLiteral("pixelFormat"),
                                                    static_cast<int>(bmdFormat8BitBGRA)).toInt());
            mOriginalConfig.legalRange  = s.value(QStringLiteral("legalRange"), true).toBool();
            mOriginalConfig.linkConfig  = static_cast<BMDLinkConfiguration>(
                                            s.value(QStringLiteral("linkConfig"),
                                                    static_cast<int>(bmdLinkConfigurationSingleLink)).toInt());
            mHasOriginal = true; // enables pre-population; configChanged() is irrelevant when not running
        }
        s.endGroup();
    }

    mDeviceCombo = new QComboBox(this);
    mModeCombo   = new QComboBox(this);
    mFormatCombo = new QComboBox(this);
    mLinkCombo   = new QComboBox(this);
    mLegalCheck  = new QCheckBox(tr("Legal range (SMPTE 64–940 / 64–960)"), this);
    mLinkLabel   = new QLabel(tr("SDI link:"), this);
    mStatusLabel = new QLabel(this);
    mStatusLabel->setWordWrap(true);

    mLinkCombo->addItem(tr("Single link"), static_cast<int>(bmdLinkConfigurationSingleLink));
    mLinkCombo->addItem(tr("Dual link"),   static_cast<int>(bmdLinkConfigurationDualLink));
    mLinkCombo->setMinimumContentsLength(28);
    mLinkCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    mFormatCombo->setMinimumContentsLength(28);
    mFormatCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    auto *form = new QFormLayout;
    form->addRow(tr("Device:"),      mDeviceCombo);
    form->addRow(tr("Mode:"),        mModeCombo);
    form->addRow(tr("Pixel format:"),mFormatCombo);
    form->addRow(mLinkLabel,         mLinkCombo);
    form->addRow(QString(),          mLegalCheck);
    form->addRow(mStatusLabel);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        const DkOutputConfig c = config();

        // Persist for future sessions
        QSettings s;
        s.beginGroup(QStringLiteral("DeckLinkOutput"));
        s.setValue(QStringLiteral("deviceName"),   mDeviceCombo->currentText());
        s.setValue(QStringLiteral("displayMode"),  static_cast<quint32>(c.mode.mode));
        s.setValue(QStringLiteral("pixelFormat"),  static_cast<int>(c.pixelFormat));
        s.setValue(QStringLiteral("legalRange"),   c.legalRange);
        s.setValue(QStringLiteral("linkConfig"),   static_cast<int>(c.linkConfig));
        s.endGroup();

        // configChanged() is only meaningful when the output was running when
        // the dialog opened (mHasOriginal from running state, not from QSettings).
        if (mHasOriginal && mOutput && mOutput->isRunning()) {
            mConfigChanged = c.deviceIndex != mOriginalConfig.deviceIndex
                          || c.mode.mode   != mOriginalConfig.mode.mode
                          || c.pixelFormat != mOriginalConfig.pixelFormat
                          || c.legalRange  != mOriginalConfig.legalRange
                          || c.linkConfig  != mOriginalConfig.linkConfig;
        } else {
            mConfigChanged = true;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);

    connect(mDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DkDeckLinkDialog::onDeviceChanged);
    connect(mModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DkDeckLinkDialog::onModeChanged);
    connect(mFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DkDeckLinkDialog::onFormatChanged);

    // Seed combos from last config if available
    populateDevices(mHasOriginal ? mOriginalConfig.deviceIndex : 0);

    if (mHasOriginal) {
        mLegalCheck->setChecked(mOriginalConfig.legalRange);
        const int li = mLinkCombo->findData(static_cast<int>(mOriginalConfig.linkConfig));
        if (li >= 0)
            mLinkCombo->setCurrentIndex(li);
    } else {
        mLegalCheck->setChecked(true);
    }
}

// -----------------------------------------------------------------------
// Populate helpers
// -----------------------------------------------------------------------

void DkDeckLinkDialog::populateDevices(int selectIndex)
{
    {
        QSignalBlocker b(mDeviceCombo);
        mDeviceCombo->clear();
        const QStringList devices = DkDeckLinkOutput::enumerateDevices();
        if (devices.isEmpty()) {
            mStatusLabel->setText(tr("No DeckLink devices found."));
            return;
        }
        for (const QString &name : devices)
            mDeviceCombo->addItem(name);

        // When loaded from QSettings, match by device name (index can shift if
        // hardware is added/removed between sessions).
        if (!mSavedDeviceName.isEmpty()) {
            for (int i = 0; i < mDeviceCombo->count(); ++i) {
                if (mDeviceCombo->itemText(i) == mSavedDeviceName) {
                    selectIndex = i;
                    break;
                }
            }
        }

        mDeviceCombo->setCurrentIndex(
            std::clamp(selectIndex, 0, mDeviceCombo->count() - 1));
    }

    const int dev = mDeviceCombo->currentIndex();
    const BMDDisplayMode selMode = mHasOriginal ? mOriginalConfig.mode.mode : bmdModeUnknown;
    const BMDPixelFormat selFmt  = mHasOriginal ? mOriginalConfig.pixelFormat : bmdFormat8BitBGRA;
    populateModes(dev, selMode);
    updateLinkVisibility(dev);

    // After modes are ready, set format
    {
        const int mi = mModeCombo->currentIndex();
        const BMDDisplayMode dm = (mi >= 0 && mi < mModes.size())
                                  ? mModes[mi].mode : bmdModeUnknown;
        populateFormats(dev, dm, selFmt);
    }
}

void DkDeckLinkDialog::populateModes(int deviceIndex, BMDDisplayMode selectMode)
{
    QSignalBlocker b(mModeCombo);
    mModeCombo->clear();
    mModes = DkDeckLinkOutput::enumerateModes(deviceIndex);

    if (mModes.isEmpty()) {
        mStatusLabel->setText(tr("No supported output modes."));
        return;
    }

    int selectIdx = 0;
    for (int i = 0; i < mModes.size(); ++i) {
        const DkDisplayModeInfo &m = mModes[i];
        const double fps = static_cast<double>(m.timeScale) / static_cast<double>(m.frameDuration);
        mModeCombo->addItem(QStringLiteral("%1  (%2×%3 @ %4 fps)")
                            .arg(m.name)
                            .arg(m.width)
                            .arg(m.height)
                            .arg(fps, 0, 'f', 2));
        if (m.mode == selectMode)
            selectIdx = i;
    }
    mModeCombo->setCurrentIndex(selectIdx);
    mStatusLabel->clear();
}

void DkDeckLinkDialog::populateFormats(int deviceIndex, BMDDisplayMode displayMode,
                                        BMDPixelFormat selectFormat)
{
    QSignalBlocker b(mFormatCombo);
    mFormatCombo->clear();
    mFormats = DkDeckLinkOutput::enumerateFormats(deviceIndex, displayMode);

    if (mFormats.isEmpty()) {
        mFormatCombo->addItem(tr("(none)"));
        updateLevelsVisibility();
        return;
    }

    int selectIdx = 0;
    for (int i = 0; i < mFormats.size(); ++i) {
        mFormatCombo->addItem(mFormats[i].label);
        if (mFormats[i].format == selectFormat)
            selectIdx = i;
    }
    mFormatCombo->setCurrentIndex(selectIdx);
    updateLevelsVisibility();
}

void DkDeckLinkDialog::updateLinkVisibility(int deviceIndex)
{
    const bool dual = DkDeckLinkOutput::supportsDualLink(deviceIndex);
    mLinkLabel->setVisible(dual);
    mLinkCombo->setVisible(dual);
    if (!dual)
        mLinkCombo->setCurrentIndex(0); // force single link when not supported
}

void DkDeckLinkDialog::updateLevelsVisibility()
{
    // Legal/full range applies to all formats (BGRA8 scales to 16-235 per channel).
    const int fi = mFormatCombo->currentIndex();
    const bool hasLevels = (fi >= 0 && fi < mFormats.size());
    mLegalCheck->setEnabled(hasLevels);
}

// -----------------------------------------------------------------------
// Slot handlers
// -----------------------------------------------------------------------

void DkDeckLinkDialog::onDeviceChanged(int index)
{
    if (index < 0)
        return;
    populateModes(index, bmdModeUnknown);
    updateLinkVisibility(index);

    const int mi = mModeCombo->currentIndex();
    const BMDDisplayMode dm = (mi >= 0 && mi < mModes.size())
                              ? mModes[mi].mode : bmdModeUnknown;
    populateFormats(index, dm, bmdFormat8BitBGRA);
}

void DkDeckLinkDialog::onModeChanged(int index)
{
    if (index < 0 || index >= mModes.size())
        return;
    const int devIdx = mDeviceCombo->currentIndex();
    const BMDDisplayMode dm = mModes[index].mode;
    const BMDPixelFormat curFmt = (mFormatCombo->currentIndex() >= 0
                                   && mFormatCombo->currentIndex() < mFormats.size())
                                  ? mFormats[mFormatCombo->currentIndex()].format
                                  : bmdFormat8BitBGRA;
    populateFormats(devIdx, dm, curFmt);
}

void DkDeckLinkDialog::onFormatChanged(int /*index*/)
{
    updateLevelsVisibility();
}

// -----------------------------------------------------------------------
// Accessors
// -----------------------------------------------------------------------

DkOutputConfig DkDeckLinkDialog::config() const
{
    DkOutputConfig cfg;
    cfg.deviceIndex = mDeviceCombo->currentIndex();

    const int mi = mModeCombo->currentIndex();
    if (mi >= 0 && mi < mModes.size())
        cfg.mode = mModes[mi];

    const int fi = mFormatCombo->currentIndex();
    if (fi >= 0 && fi < mFormats.size())
        cfg.pixelFormat = mFormats[fi].format;
    else
        cfg.pixelFormat = bmdFormat8BitBGRA;

    cfg.legalRange = mLegalCheck->isChecked();

    const int li = mLinkCombo->currentIndex();
    cfg.linkConfig = (li >= 0)
                   ? static_cast<BMDLinkConfiguration>(mLinkCombo->itemData(li).toInt())
                   : bmdLinkConfigurationSingleLink;

    return cfg;
}

} // namespace nmc
