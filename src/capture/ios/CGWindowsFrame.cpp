#include <ApplicationServices/ApplicationServices.h>
#include <iostream>
#include "SamplesProvider.h"

bool FrameGetter::windowsFrame(FrameBuffer * fb, SPType type, size_t handler, SPImageType imgtype)
{
    CGImageRef imageRef;
    imageRef = CGWindowListCreateImage (CGRectNull, kCGWindowListOptionIncludingWindow,
                                        static_cast<uint32_t>(handler),
                                        kCGWindowImageBoundsIgnoreFraming);


    if (!imageRef) {
        return false; // this happens when the monitors change.
    }
    auto width = CGImageGetWidth(imageRef);
    auto height = CGImageGetHeight(imageRef);

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

    fb->setDataPerRow((unsigned char *)buf, width, height, bytesperrow, imgtype);

    CFRelease(rawdatas);
    CGImageRelease(imageRef);
    return true;
}

bool FrameGetter::exportAllFrameGetter(FrameBuffer * fb, SPType type, size_t handler)
{
    return FrameGetter::windowsFrame(fb, type, handler, SPImageType::SP_IMAGE_RGBA);
}
