#include "DkDeckLinkPlugin.h"
#include "DkDeckLinkViewPort.h"
#include "DkDeckLinkDialog.h"

#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>

namespace nmc
{

DkDeckLinkPlugin::DkDeckLinkPlugin(QObject *parent)
    : QObject(parent)
{
}

DkDeckLinkPlugin::~DkDeckLinkPlugin()
{
    hideStopToolBar();
    if (mOutput) {
        mOutput->stopOutput();
        delete mOutput;
    }
}

bool DkDeckLinkPlugin::createViewPort(QWidget *parent)
{
    if (!mOutput)
        mOutput = new DkDeckLinkOutput();

    mViewPort = new DkDeckLinkViewPort(mOutput, parent);
    return true;
}

DkPluginViewPort *DkDeckLinkPlugin::getViewPort()
{
    return mViewPort;
}

void DkDeckLinkPlugin::setVisible(bool visible)
{
    if (!mViewPort)
        return;

    if (visible) {
        DkDeckLinkDialog dlg(mOutput, getMainWindow());
        if (dlg.exec() != QDialog::Accepted) {
            mViewPort->closePlugin();
            return;
        }

        const DkOutputConfig cfg = dlg.config();

        if (!mOutput->isRunning() || dlg.configChanged()) {
            mOutput->stopOutput();
            if (!mOutput->startOutput(cfg)) {
                QMessageBox::critical(getMainWindow(),
                                      tr("DeckLink Output"),
                                      tr("Failed to start output:\n%1").arg(mOutput->lastError()));
                mViewPort->closePlugin();
                return;
            }
        }

        // Immediately send whatever image is currently displayed so the output
        // shows something before the next image-change event fires.
        if (mViewPort->lastContainer())
            mOutput->sendImage(mViewPort->lastContainer()->image());

        showStopToolBar();
    } else {
        if (mOutput)
            mOutput->stopOutput();
        hideStopToolBar();
    }

    mViewPort->setVisible(visible);
}

void DkDeckLinkPlugin::showStopToolBar()
{
    QMainWindow *mw = getMainWindow();
    if (!mw)
        return;

    if (!mStopBar) {
        mStopBar = new QToolBar(tr("DeckLink"), mw);
        mStopBar->setObjectName(QStringLiteral("DeckLinkStopBar"));
        mStopBar->setMovable(false);

        auto *lbl = new QLabel(QStringLiteral("  ● DeckLink Live  "), mStopBar);
        lbl->setStyleSheet(QStringLiteral("color: #cc3333; font-weight: bold;"));
        mStopBar->addWidget(lbl);

        auto *stopBtn = new QPushButton(tr("Stop Output"), mStopBar);
        mStopBar->addWidget(stopBtn);

        connect(stopBtn, &QPushButton::clicked, this, [this] {
            if (mOutput)
                mOutput->stopOutput();
            if (mViewPort)
                mViewPort->closePlugin(); // tells nomacs to deactivate the plugin
        });

        mw->addToolBar(Qt::TopToolBarArea, mStopBar);
    }

    mStopBar->setVisible(true);
}

void DkDeckLinkPlugin::hideStopToolBar()
{
    if (!mStopBar)
        return;

    QMainWindow *mw = getMainWindow();
    if (mw)
        mw->removeToolBar(mStopBar);

    delete mStopBar;
    mStopBar = nullptr;
}

} // namespace nmc
