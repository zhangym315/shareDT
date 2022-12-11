#include "RemoteGetter.h"
#include "Buffer.h"
#include "ExportAll.h"
#include "Logger.h"
#include "Converter.h"
#include "SubFunction.h"

#include <QImage>

void RemoteGetter::send()
{
    CircleWRBuf<FrameBuffer>  cwb(2);
    MonitorVectorProvider mvp;
    FrameBuffer * fb;
    CapPoint cp(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());

    LOGGER.info() << "RemoteGetter::send";
    for (auto & m : mvp.get()) {
        ExportAll ea(SP_MONITOR, m.getId());

        if ((fb = ea.getFrameBuffer(cwb)) == nullptr) continue;

        QImage im(fb->getData(), fb->getWidth(), fb->getHeight(),
                  QImage::Format::Format_RGBX8888);

        RemoteGetterMsg msg(_replyW, _replyH);
        im = im.scaled((int)_replyW, (int)_replyH);
        msg.dataLen = im.sizeInBytes();

        ::strcpy(msg.name, m.getName().c_str());
        ::strcpy(msg.cmdArgs, SHAREDT_SERVER_COMMAND_CAPTURE);
        ::strcat(msg.cmdArgs, " -c mon -i ");
        ::strcat(msg.cmdArgs, std::to_string(m.getId()).c_str());

        msg.convert();
        _sk->sendBytes((const unsigned char *)&msg, sizeof(msg));

        String head;
        head << "w=" << fb->getWidth() << ",h=" << fb->getHeight();
        LOGGER.info() << "Sent for monitors capture number of bytes=" << im.sizeInBytes() <<
                    " sent=" << _sk->sendBytes(im.bits(), im.sizeInBytes());

        //set smallest width and height
        if (cp.getX() > fb->getWidth() && cp.getY() > fb->getHeight()) {
            cp.setX((int) fb->getWidth());
            cp.setY((int) fb->getHeight());
        }

        fb->setUsed();
    }

    WindowVectorProvider wvp(-1);
    for (const auto & w : wvp.get()) {
        if (ExportAll::filterExportWinName(w.getName())) {
            LOGGER.info() << "Skipped fresh window_id=" << w.getHandler() << " name=\"" << w.getName();
            continue;
        }

        ExportAll ea(SP_WINDOW, w.getHandler());
        // filter out the unnecessary window
        if ((fb = ea.getFrameBuffer(cwb)) == nullptr ||
            fb->getWidth() < cp.getX() / 8 ||
            fb->getHeight() < cp.getY() / 8) {
            continue;
        }

        QImage im(fb->getData(), fb->getWidth(), fb->getHeight(),
                  QImage::Format::Format_RGBX8888);

        RemoteGetterMsg msg(_replyW, _replyH);
        im = im.scaled((int)_replyW, (int)_replyH);
        msg.dataLen = im.sizeInBytes();

        ::strcpy(msg.name, w.getName().c_str());
        ::strcpy(msg.cmdArgs, SHAREDT_SERVER_COMMAND_CAPTURE);
        ::strcat(msg.cmdArgs, " -c win -h ");
        ::strcat(msg.cmdArgs, std::to_string(w.getHandler()).c_str());

        msg.convert();

        _sk->sendBytes((const unsigned char *)&msg, sizeof(msg));
        LOGGER.info() << "Sent for windows capture number of bytes=" << im.sizeInBytes() <<
                      " sent=" << _sk->sendBytes(im.bits(), im.sizeInBytes());
        fb->setUsed();
    }

    LOGGER.info() << "RemoteGetter::send finished";
}

void RemoteGetterMsg::convert() {
    w = Converter::toLittleEndian(w);
    h = Converter::toLittleEndian(h);
    dataLen = Converter::toLittleEndian(dataLen);
}
