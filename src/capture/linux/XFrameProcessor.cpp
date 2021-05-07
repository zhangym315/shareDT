#include <X11/Xutil.h>
#include <assert.h>
#include <vector>
#include "XFrameProcessor.h"

static X11FrameProcessor * x11FP = NULL;

/* nothing to do with linux */
void FrameProcessorWrap::init() {
}

void FrameProcessorWrap::pause() {
}

void FrameProcessorWrap::resume() {
}

bool X11FrameProcessor::init()
{
    bool ret = true;

    SelectedDisplay = XOpenDisplay(NULL);
    if(!SelectedDisplay) {
        return false;
    }


    ShmInfo = std::make_unique<XShmSegmentInfo>();

    if(_type == SP_MONITOR) {
        int scr = _mon->getId();
        XImage_ = XShmCreateImage(SelectedDisplay,
                                  DefaultVisual(SelectedDisplay, scr),
                                  DefaultDepth(SelectedDisplay, scr),
                                  ZPixmap,
                                  NULL,
                                  ShmInfo.get(),
                                  _mon->getOrgWidth (),
                                  _mon->getOrgHeight());
    } else if( _type == SP_PARTIAL) {
        int scr = _mon->getId();
        XImage_ = XShmCreateImage(SelectedDisplay,
                                  DefaultVisual(SelectedDisplay, scr),
                                  DefaultDepth(SelectedDisplay, scr),
                                  ZPixmap,
                                  NULL,
                                  ShmInfo.get(),
                                  _bounds->getWidth(),
                                  _bounds->getHeight ());
    } else if ( _type == SP_WINDOW ) {
            SelectedWindow = _win->getHandler ();

            int scr = XDefaultScreen(SelectedDisplay);
            XImage_ = XShmCreateImage(SelectedDisplay,
                                  DefaultVisual(SelectedDisplay, scr),
                                  DefaultDepth(SelectedDisplay, scr),
                                  ZPixmap,
                                  NULL,
                                  ShmInfo.get(),
                                  _win->getWidth(),
                                  _win->getHeight ());
    } else return false;

    ShmInfo->shmid = shmget(IPC_PRIVATE, XImage_->bytes_per_line * XImage_->height, IPC_CREAT | 0777);

    ShmInfo->readOnly = False;
    ShmInfo->shmaddr = XImage_->data = (char*)shmat(ShmInfo->shmid, 0, 0);
std::cout <<  "XImage_ format: " << XImage_->format;
    XShmAttach(SelectedDisplay, ShmInfo.get());

    return ret;
}

bool X11FrameProcessor::ProcessFrame(FrameBuffer * fb)
{

    if(_type == SP_MONITOR) {
        if(!XShmGetImage(SelectedDisplay,
                 RootWindow(SelectedDisplay, _mon->getId ()),
                 XImage_,
                 0,0,
                 AllPlanes)) {
            return false;
        }
        fb->setDataPerRow((unsigned char*)XImage_->data, _mon->getWidth(),
                          _mon->getHeight(), XImage_->bytes_per_line,
                          FrameProcessorWrap::instance()->getImageType());
    } else if(_type == SP_PARTIAL) {
        if(!XShmGetImage(SelectedDisplay,
                       RootWindow(SelectedDisplay, _mon->getId ()),
                       XImage_,
                       _bounds->getLT().getX(),
                       _bounds->getLT().getY(),
                       AllPlanes)) {
            return false;
        }
        fb->setDataPerRow((unsigned char*)XImage_->data,_bounds->getWidth(),
                          _bounds->getHeight(), XImage_->bytes_per_line,
                          FrameProcessorWrap::instance()->getImageType());
    } else if ( _type == SP_WINDOW ) {
        XWindowAttributes wndattr;

        if(XGetWindowAttributes(SelectedDisplay, SelectedWindow, &wndattr) ==0){
            return false ;//window might not be valid
        }

        if(wndattr.width != _win->getWidth() || wndattr.height !=_win->getHeight ()){
            return false;//window size changed. This will rebuild everything
        }

        if(!XShmGetImage(SelectedDisplay,
                         _win->getHandler(),
                         XImage_,
                         0,
                         0,
                         AllPlanes)) {
            return false;
        }
        fb->setDataPerRow((unsigned char*)XImage_->data,
                          _win->getWidth(), _win->getHeight(),
                          XImage_->bytes_per_line,
                          FrameProcessorWrap::instance()->getImageType());
    }

    return true;
}

void CircleWriteThread::init() {
    if(_type == SP_MONITOR)
        x11FP = new X11FrameProcessor(_mon);
    else if (_type == SP_PARTIAL)
        x11FP = new X11FrameProcessor(_bounds, _mon);
    else if (_type == SP_WINDOW) {
        x11FP = new X11FrameProcessor(_win);
    }
}

bool CircleWriteThread::WindowsFrame(FrameBuffer * fb) {

    if(x11FP)
        return x11FP->ProcessFrame(fb);
    else {
        std::cerr << "CircleWriteThread::WindowsFrame x11FB is NULL" << std::endl;
        return false;
    }

}