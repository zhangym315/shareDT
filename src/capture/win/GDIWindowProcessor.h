#ifndef SHAREDT_GDIWINDOWPROCESSOR_H
#define SHAREDT_GDIWINDOWPROCESSOR_H

#include "SamplesProvider.h"
#include "ImageRect.h"
#include "GDIHelper.h"

class GDIFrameProcessor {
  public:
    /* monitor */
    GDIFrameProcessor(CapMonitor * mon) : _mon(mon) {
        _type = SP_MONITOR;
        if(!init())
            _mon->setInValid ();
    }

    /* partial */
    GDIFrameProcessor(CapImageRect * bound, CapMonitor * mon) : _bounds(bound) {
        _type = SP_PARTIAL;
        _mon = mon;
        if(!init())
            _bounds->setInvalid();
    }

    /* window */
    GDIFrameProcessor(CapWindow * win) : _win(win) {
        _type = SP_WINDOW;
        if(!init())
         _win->setInvalid();
    }

    bool init();
    bool ProcessFrame(FrameBuffer * fb);

  private:
    HDCWrapper MonitorDC;
    HDCWrapper CaptureDC;
    HBITMAPWrapper CaptureBMP;
    CapMonitor SelectedMonitor;
    HWND SelectedWindow;

    CapMonitor *   _mon;
    CapImageRect * _bounds;
    CapWindow    * _win;
    SPType         _type;
};

#endif //SHAREDT_GDIWINDOWPROCESSOR_H
