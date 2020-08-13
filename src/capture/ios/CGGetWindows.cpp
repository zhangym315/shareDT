#include "WindowProcessor.h"
#include <algorithm>
#include <ApplicationServices/ApplicationServices.h>

static char * CapCFStringCopyUTF8String(CFStringRef aString) {
    if (aString == NULL) {
            return NULL;
        }

    CFIndex length = CFStringGetLength(aString);
    CFIndex maxSize =
    CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    char *buffer = (char *)malloc(maxSize);
    if (CFStringGetCString(aString, buffer, maxSize,
                       kCFStringEncodingUTF8)) {
        return buffer;
    }

    free(buffer); // If we failed
    return NULL;
}

/*
 * Get the window lists from the process id
 */
void WindowVectorProvider::CapGetWindows()
{
    CGDisplayCount count=0;
    CGGetActiveDisplayList(0, 0, &count);
    std::vector<CGDirectDisplayID> displays;
    displays.resize(count);
    CGGetActiveDisplayList(count, displays.data(), &count);
    auto xscale=1.0f;
    auto yscale = 1.0f;

    for(auto  i = 0; i < count; i++) {
        //only include non-mirrored displays
        if(CGDisplayMirrorsDisplay(displays[i]) == kCGNullDirectDisplay){
            auto dismode =CGDisplayCopyDisplayMode(displays[i]);
            auto scaledsize = CGDisplayBounds(displays[i]);

            auto pixelwidth = CGDisplayModeGetPixelWidth(dismode);
            auto pixelheight = CGDisplayModeGetPixelHeight(dismode);

            CGDisplayModeRelease(dismode);

            if(scaledsize.size.width !=pixelwidth){//scaling going on!
                xscale = static_cast<float>(pixelwidth)/static_cast<float>(scaledsize.size.width);
            }
            if(scaledsize.size.height !=pixelheight){//scaling going on!
                yscale = static_cast<float>(pixelheight)/static_cast<float>(scaledsize.size.height);
            }
            break;
        }
    }

    auto windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    WindowVector ret;
    CFIndex numWindows = CFArrayGetCount(windowList );

    for( int i = 0; i < (int)numWindows; i++ ) {
        uint32_t windowid=0;
        auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, i));
        auto cfwindowname = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kCGWindowName));

        // window name is NULL, no need to add to list
        if(cfwindowname == NULL) continue;
        String name(CapCFStringCopyUTF8String(cfwindowname));

        if(!_getAll && name.length() == 0) continue;

        /* check if we need to push back the new _pid */
        pid_t curPid;
        auto cfwindowpid = static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCGWindowOwnerPID));
        CFNumberGetValue(cfwindowpid, kCFNumberSInt64Type, &curPid);
        if(_pid != -1 && _pid != curPid) continue;

        CFNumberGetValue(static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCGWindowNumber)), kCFNumberIntType, &windowid);

        auto dims =static_cast<CFDictionaryRef>(CFDictionaryGetValue(dict,kCGWindowBounds));
        CGRect rect;
        CGRectMakeWithDictionaryRepresentation(dims, &rect);

        CapPoint offset(rect.origin.x, rect.origin.y);
        CapPoint size (rect.size.width * xscale, rect.size.height* yscale);
        CapWindow w(static_cast<size_t>(windowid), offset, size, name, curPid);
        ret.push_back(w);
    }
    CFRelease(windowList);

    /* if size changed, need to change the window vector */
    if(ret.size() != _wins.size()) {
        {
            std::lock_guard<std::mutex> lk (_winMtx);
            _wins.swap(ret);
        }
        ret.clear();
    }
    return;
}
