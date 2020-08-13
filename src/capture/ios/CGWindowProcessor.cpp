#include "CGWindowProcessor.h"
//#include <ApplicationServices/ApplicationServices.h>

WINPROCESSOR_RETURN CGFrameProcessor::Init(CapWindow &window)
{
    auto ret = WINPROCESSOR_RETURN_SUCCESS;
//    Data = data;
    return ret;
}

WINPROCESSOR_RETURN CGFrameProcessor::ProcessFrame(const CapWindow &window)
{
    auto Ret = WINPROCESSOR_RETURN_SUCCESS;
#if 0

    auto imageRef = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, static_cast<uint32_t>(window.Handle),
                                            kCGWindowImageBoundsIgnoreFraming);
    if (!imageRef)
        return DUPL_RETURN_ERROR_EXPECTED; // this happens when the monitors change.

    auto width = CGImageGetWidth(imageRef);
    auto height = CGImageGetHeight(imageRef);

    if (width != window.Size.x || height != window.Size.y) {
            CGImageRelease(imageRef);
            return DUPL_RETURN_ERROR_EXPECTED; // this happens when the window sizes change.
        }
    auto prov = CGImageGetDataProvider(imageRef);
    if (!prov) {
            CGImageRelease(imageRef);
            return DUPL_RETURN_ERROR_EXPECTED;
        }
    auto bytesperrow = CGImageGetBytesPerRow(imageRef);
    auto bitsperpixel = CGImageGetBitsPerPixel(imageRef);
    // right now only support full 32 bit images.. Most desktops should run this as its the most efficent
    assert(bitsperpixel == sizeof(ImageBGRA) * 8);

    auto rawdatas = CGDataProviderCopyData(prov);
    auto buf = CFDataGetBytePtr(rawdatas);

    ProcessCapture(Data->WindowCaptureData, *this, window, buf, bytesperrow);

    CFRelease(rawdatas);
    CGImageRelease(imageRef);
#endif
    return Ret;
}
