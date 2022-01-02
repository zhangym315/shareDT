#include "SamplesProvider.h"
#include "GDIWindowProcessor.h"
#include "WindowProcessor.h"
#include "Logger.h"
#include "ScopedPrt.h"

static GDIFrameProcessor * gdiFP = NULL;

/* nothing to do with windows */
void FrameProcessorWrap::init() {
}

void FrameProcessorWrap::pause() {
}

void FrameProcessorWrap::resume() {
}

bool GDIFrameProcessor::init() {
    if(_type == SP_MONITOR || _type == SP_PARTIAL) {
        MonitorDC.DC = CreateDCA(_mon->getName().c_str(), NULL, NULL, NULL);
        CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
        CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC,
                               (_type == SP_MONITOR) ? _mon->getOrgWidth() : _bounds->getWidth(),
                               (_type == SP_MONITOR) ? _mon->getOrgHeight(): _bounds->getHeight());
        if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
            LOGGER.error() << "Create MonitorDC/CaptureDC/CaptureDMP failed";
            return false;
        }
    } else if (_type == SP_WINDOW) {
        // this is needed to fix AERO BitBlt capturing issues
        ANIMATIONINFO str;
        str.cbSize = sizeof(str);
        str.iMinAnimate = 0;
        SystemParametersInfo(SPI_SETANIMATION, sizeof(str), (void *)&str, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

        MonitorDC.DC = GetWindowDC((HWND)_win->getHandler());
        CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);

        CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, _win->getWidth(), _win->getHeight());

        if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
            LOGGER.error() << "Create MonitorDC/CaptureDC/CaptureDMP failed" ;
            return false;
        }
    }
    return true;
}

bool GDIFrameProcessor::ProcessFrame(FrameBuffer * fb) {
    if(_type == SP_MONITOR || _type == SP_PARTIAL) {
        int width, height;
        if(_type == SP_MONITOR) {
            width  = _mon->getOrgWidth();
            height = _mon->getOrgHeight();
        } else {
            width  = _bounds->getWidth();
            height = _bounds->getHeight();
        }

        fb->setWidthHeight(width, height);
        auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);

        if (_type == SP_MONITOR && BitBlt(CaptureDC.DC, 0, 0, width,
                    height, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
            SelectObject(CaptureDC.DC, originalBmp);
            LOGGER.error() << "Failed on CaptureDC.DC" ;
            return false;
        } else if (_type == SP_PARTIAL && BitBlt(CaptureDC.DC,0, 0,
                    width, height, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
            SelectObject(CaptureDC.DC, originalBmp);
            LOGGER.error() << "Failed on CaptureDC.DC" ;
            return false;
        } else {
            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);

            bi.biWidth = width;
            bi.biHeight = -height;
            bi.biPlanes = 1;
            bi.biBitCount = sizeof(ImageBGRA) * 8; // always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = ((width * bi.biBitCount + 31) / (sizeof(ImageBGRA) * 8)) * sizeof(ImageBGRA)  * height;
            fb->reSet(width * height * 4);
            GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)height, fb->getData(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
            SelectObject(CaptureDC.DC, originalBmp);
            fb->ConvertBGRA2RGBA();
        }
    } else if(_type == SP_WINDOW) {
        WindowDimensions windowrect = GetWindowRect((HWND)_win->getHandler());
        int l = windowrect.ClientRect.left;
        int t = windowrect.ClientRect.top;
        int r = windowrect.ClientRect.right;
        int b = windowrect.ClientRect.bottom;
        int width  = r - l;
        int height = b - t;

        // Selecting an object into the specified DC
        auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);
        auto left = -windowrect.ClientBorder.left;
        auto top = -windowrect.ClientBorder.top;
        fb->setWidthHeight(width, height);

        if (BitBlt(CaptureDC.DC, left, top, r, b, MonitorDC.DC, 0, 0, SRCCOPY) == FALSE) {
            LOGGER.error() << "Failed on BitBlt: " << l << "  " << t << " " << r << " " << b << " " ;
            SelectObject(CaptureDC.DC, originalBmp);
            return false;
        }
        else {

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);

            bi.biWidth = width;
            bi.biHeight = -height;
            bi.biPlanes = 1;
            bi.biBitCount = sizeof(ImageBGRA) * 8; // always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = (width * bi.biBitCount + 31) / ((sizeof(ImageBGRA) * 8)) * sizeof(ImageBGRA)  * height;
            fb->reSet(width * height * 4);
            GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)height, fb->getData(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
            fb->ConvertBGRA2RGBA();
            SelectObject(CaptureDC.DC, originalBmp);
        }
    }
    return true;
}

void CircleWriteThread::init() {
    if(_type == SP_MONITOR)
        gdiFP = new GDIFrameProcessor(_mon);
    else if (_type == SP_PARTIAL)
        gdiFP = new GDIFrameProcessor(_bounds, _mon);
    else if (_type == SP_WINDOW) {
        gdiFP = new GDIFrameProcessor(_win);
    }
}

bool FrameGetter::WindowsFrame(FrameBuffer * fb, SPType type, size_t handler) {
    (void) type; (void) handler;
    if(gdiFP)
        return gdiFP->ProcessFrame(fb);
    else {
        LOGGER.error() << "FrameGetter::WindowsFrame gdiFP is NULL" ;
        return false;
    }
}

bool FrameGetter::ExportAllFrameGetter(FrameBuffer * fb, SPType type, size_t handler)
{
    ScopedPtr<GDIFrameProcessor> fp;
    CapMonitor cm;
    CapWindow  cw;

    if (type == SP_MONITOR)
    {
        cm = CapMonitor::getById((int)handler);
        if (!cm.isValid()) {
            LOGGER.error() << "FrameGetter::ExportAllFrameGetter failed to get CapMonitor for monitor id=" << handler;
            return false;
        }
        fp.reset(new GDIFrameProcessor(&cm));
    } else if  (type == SP_WINDOW) {
        cw = CapWindow::getWinById(handler);
        if (!cw.isValid()) {
            LOGGER.error() << "FrameGetter::ExportAllFrameGetter failed to get CapWindow for window handler=" << handler;
            return false;
        }
        fp.reset(new GDIFrameProcessor(&cw));
    } else {
        LOGGER.error() << "FrameGetter::ExportAllFrameGetter doesnot support capture type=" << type;
        return false;
    }

    return fp->ProcessFrame(fb);
}
