#include "RemoteGetter.h"
#include "Buffer.h"
#include "ExportAll.h"
#include "Logger.h"

#include <QImage>

void RemoteGetter::send()
{
    CircleWRBuf<FrameBuffer>  cwb(2);
    MonitorVectorProvider mvp;
    FrameBuffer * fb;
    LOGGER.info() << "RemoteGetter::send";
    for (auto & m : mvp.get()) {
        ExportAll ea(SP_MONITOR, m.getId());

        if ((fb = ea.getFrameBuffer(cwb)) == nullptr) continue;

        QImage im(fb->getData(), fb->getWidth(), fb->getHeight(),
                  QImage::Format::Format_RGBX8888);
        im = im.scaled(140, 110);

        String head;

        head << "w=" << fb->getWidth() << ",h=" << fb->getHeight();
        LOGGER.info() << "Sent number of bytes=" << im.sizeInBytes() <<
                    " sent=" << _sk->sendBytes(im.bits(), im.sizeInBytes());
    }
    LOGGER.info() << "RemoteGetter::send finished";
}
