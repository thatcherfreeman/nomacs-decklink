#include "DkDeckLinkViewPort.h"
#include "DkDeckLinkOutput.h"
#include "DkImageContainer.h"

namespace nmc
{

DkDeckLinkViewPort::DkDeckLinkViewPort(DkDeckLinkOutput *output, QWidget *parent)
    : DkPluginViewPort(parent)
    , mOutput(output)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setVisible(false);
}

void DkDeckLinkViewPort::updateImageContainer(QSharedPointer<DkImageContainerT> imgC)
{
    mLastContainer = imgC; // always cache so the plugin can send it immediately on start
    if (!mOutput || !mOutput->isRunning() || !imgC)
        return;
    mOutput->sendImage(imgC->image());
}

} // namespace nmc
