#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include "SamplesProvider.h"
#include <iostream>

bool CircleWriteThread::WindowsFrame(FrameBuffer * fb)
{

    CFArrayRef windows ;
//    windows->append(100);
//    auto cfArray = CFArrayCreate(kCFAllocatorDefault, windows, windows->count, nullptr)
    CFMutableArrayRef array;
    array = CFArrayCreateMutable(kCFAllocatorDefault,
                                 0,
                                 &kCFTypeArrayCallBacks);
    CFArrayAppendValue(array, CFSTR("1000"));
    CFArrayAppendValue(array, CFSTR("257"));
    CFArrayAppendValue(array, CFSTR("628"));
    std::vector<size_t>::iterator  it;
    for(it = _win->getAll().begin (); it < _win->getAll().end(); it++)
    {
//        std::cout << " it: " << *it << std::endl;
//        CFArrayAppendValue(array, CFSTR(std::to_string(*it)));
    }

    CGImageRef imageRef;
    if (_win->getWinType () == SP_WIN_HANDLER)
        imageRef = CGWindowListCreateImage (CGRectNull, kCGWindowListOptionIncludingWindow,
            static_cast<uint32_t>(_win->getHandler ()), kCGWindowImageBoundsIgnoreFraming);
    else imageRef = CGWindowListCreateImageFromArray(CGRectNull, (CFArrayRef)array, kCGWindowImageBoundsIgnoreFraming);


    if (!imageRef) {
        return false; // this happens when the monitors change.
    }
    auto width = CGImageGetWidth(imageRef);
    auto height = CGImageGetHeight(imageRef);

    if (width != _win->getWidth () || height != _win->getHeight ()) {
        CGImageRelease(imageRef);
        std::cout << "get Windows width and height not match " << std::endl;
        return false;
    }

    auto prov = CGImageGetDataProvider(imageRef);
    if (!prov) {
        CGImageRelease(imageRef);
        return false;
    }

    auto bytesperrow = CGImageGetBytesPerRow(imageRef);
    auto bitsperpixel = CGImageGetBitsPerPixel(imageRef);
    // right now only support full 32 bit images.. Most desktops should run this as its the most efficent
    assert(bitsperpixel == sizeof(CapImageBGRA) * 8);

    auto rawdatas = CGDataProviderCopyData(prov);
    auto buf = CFDataGetBytePtr(rawdatas);

    fb->setDataPerRow((unsigned char *)buf, _win->getWidth(), _win->getHeight(), bytesperrow);

    CFRelease(rawdatas);
    CGImageRelease(imageRef);
    return true;
}
