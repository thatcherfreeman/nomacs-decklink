#include "DkDeckLinkPlugin.h"
#include "DkDeckLinkViewPort.h"
#include "DkDeckLinkDialog.h"

#include <QGuiApplication>
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
            disconnect(mStateConn);
            mOutput->stopOutput();
            if (!mOutput->startOutput(cfg)) {
                QMessageBox::critical(getMainWindow(),
                                      tr("DeckLink Output"),
                                      tr("Failed to start output:\n%1").arg(mOutput->lastError()));
                mViewPort->closePlugin();
                return;
            }
        }

        mLastConfig = cfg;
        if (cfg.releaseOnFocusLoss) {
            mStateConn = connect(qApp, &QGuiApplication::applicationStateChanged,
                                 this, &DkDeckLinkPlugin::onApplicationStateChanged);
        }

        // Immediately send whatever image is currently displayed so the output
        // shows something before the next image-change event fires.
        if (mViewPort->lastContainer())
            mOutput->sendImage(mViewPort->lastContainer()->image());

        showStopToolBar();
    } else {
        disconnect(mStateConn);
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

void DkDeckLinkPlugin::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (!mOutput)
        return;

    if (state == Qt::ApplicationInactive) {
        mOutput->stopOutput();
    } else if (state == Qt::ApplicationActive && !mOutput->isRunning()) {
        if (mOutput->startOutput(mLastConfig)) {
            if (mViewPort && mViewPort->lastContainer())
                mOutput->sendImage(mViewPort->lastContainer()->image());
        }
    }
}

} // namespace nmc
