#include "ExportAll.h"
#include "SamplesProvider.h"

FrameBuffer * ExportAll::getFrameBuffer(CircWRBuf<FrameBuffer> & cwf)
{
    FrameBuffer * fb;
    cwf.setEmpty();
#ifdef __SHAREDT_IOS__
    // IOS monitor capture is different
    if (_type == SP_MONITOR) {
        const CapMonitor & cp = CapMonitor::getById(_captureId);
        if (!cp.isValid()) return nullptr;

        FrameProcessorWrap * fpw = FrameProcessorWrap::instance();
        fpw->setReInitiated();

        fpw->setMV(const_cast<CapMonitor *>(&cp), 1);
        fpw->setCFB(&cwf);

        int indicator = 0;
        while(!fpw->isReady() && ++indicator < 100) {
            std::this_thread::sleep_for(50ms);
        }
        if (!fpw->isReady()) return nullptr;

        fpw->resume();
        while ((fb = cwf.getToRead()) == nullptr && ++indicator < 200) {
            std::this_thread::sleep_for(50ms);
        }

        if (fb) fb->setWidthHeight(cp.getOrgWidth(), cp.getOrgHeight());

        return fb;
    }
#endif

    fb = cwf.getToWrite();
    return FrameGetter::exportAllFrameGetter(fb, _type, _captureId) ? fb : nullptr;
}


