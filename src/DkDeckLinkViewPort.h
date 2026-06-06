#pragma once

#include "DkPluginInterface.h"

namespace nmc
{

class DkDeckLinkOutput;

class DkDeckLinkViewPort : public DkPluginViewPort
{
    Q_OBJECT

public:
    explicit DkDeckLinkViewPort(DkDeckLinkOutput *output, QWidget *parent = nullptr);

    void updateImageContainer(QSharedPointer<DkImageContainerT> imgC) override;

    // Returns the most recently received image container (may be null).
    QSharedPointer<DkImageContainerT> lastContainer() const { return mLastContainer; }

    // Expose close so the plugin can trigger it on cancel
    void closePlugin() { emit DkPluginViewPort::closePlugin(); }

protected:
    void paintEvent(QPaintEvent *) override {}

private:
    DkDeckLinkOutput *mOutput = nullptr; // non-owning
    QSharedPointer<DkImageContainerT> mLastContainer;
};

} // namespace nmc
