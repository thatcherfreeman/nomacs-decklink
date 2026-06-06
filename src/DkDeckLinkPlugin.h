#pragma once

#include "DkPluginInterface.h"
#include "DkDeckLinkOutput.h"

#include <QToolBar>

namespace nmc
{

class DkDeckLinkViewPort;

class DkDeckLinkPlugin : public QObject, public DkViewPortInterface
{
    Q_OBJECT
    Q_INTERFACES(nmc::DkViewPortInterface)
    Q_PLUGIN_METADATA(IID "com.nomacs.ImageLounge.DkViewPortInterface/3.8"
                      FILE "DkDeckLinkPlugin.json")

public:
    explicit DkDeckLinkPlugin(QObject *parent = nullptr);
    ~DkDeckLinkPlugin() override;

    QImage image() const override { return QImage(); }

    QSharedPointer<DkImageContainer> runPlugin(
        const QString &runID = QString(),
        QSharedPointer<DkImageContainer> imgC = QSharedPointer<DkImageContainer>()) const override
    {
        return imgC;
    }

    bool closesOnImageChange() const override { return false; }
    bool hideHUD() const override { return false; }

    bool createViewPort(QWidget *parent) override;
    DkPluginViewPort *getViewPort() override;
    void setVisible(bool visible) override;

private:
    void showStopToolBar();
    void hideStopToolBar();

    DkDeckLinkViewPort *mViewPort  = nullptr;
    DkDeckLinkOutput   *mOutput    = nullptr;
    QToolBar           *mStopBar   = nullptr;
};

} // namespace nmc
