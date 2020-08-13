#ifndef _XFRAMEPROCESSOR_H_
#define _XFRAMEPROCESSOR_H_
#include <memory>
#include <X11/Xlib.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include "SamplesProvider.h"

class X11FrameProcessor {
  public:
    /* monitor */
    X11FrameProcessor(CapMonitor * mon) : _mon(mon) {
        _type = SP_MONITOR;
        if(!init())
            _mon->setInValid ();
    }

    /* partial */
    X11FrameProcessor(CapImageRect * bound, CapMonitor * mon) : _bounds(bound) {
        _type = SP_PARTIAL;
        _mon = mon;
        if(!init())
            _bounds->setInvalid();
    }

    /* window */
    X11FrameProcessor(CapWindow * win) : _win(win) {
        _type = SP_WINDOW;
        if(!init())
            _win->setInvalid();
    }

    ~X11FrameProcessor();

    bool init();
    bool ProcessFrame(FrameBuffer * fb);

  private:
    Display* SelectedDisplay=nullptr;
    XID SelectedWindow = 0;
    XImage* XImage_=nullptr;
    std::unique_ptr<XShmSegmentInfo> ShmInfo;
    CapMonitor *   _mon;
    CapImageRect * _bounds;
    CapWindow    * _win;
    SPType         _type;
};
#endif //_XFRAMEPROCESSOR_H_

