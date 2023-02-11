#include <ApplicationServices/ApplicationServices.h>
#include "WindowProcessor.h"

void MonitorVectorProvider::CapGetMonitors ()
{
//    CGDisplayCount count=0;
    //get count
//    CGGetActiveDisplayList(0, nullptr, &count);
//    displays.resize(20);
#define MAX_DISPLAYS 20
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t count;
//    uint32_t i;

    CGGetActiveDisplayList(MAX_DISPLAYS, displays, &count);

    for(auto  i = 0; i < count; i++) {
        //only include non-mirrored displays
        if(CGDisplayMirrorsDisplay(displays[i]) == kCGNullDirectDisplay){

            auto dismode =CGDisplayCopyDisplayMode(displays[i]);

            auto width = CGDisplayModeGetPixelWidth(dismode);
            auto height = CGDisplayModeGetPixelHeight(dismode);
            CGDisplayModeRelease(dismode);
            auto r = CGDisplayBounds(displays[i]);

            auto scale = static_cast<float>(width)/static_cast<float>(r.size.width);
            auto name = std::string("Monitor ") + std::to_string(displays[i]);

            CapMonitor m(_mons.size(), displays[i], width, height,
                int(r.origin.x), int(r.origin.y), int(r.size.width), int(r.size.height), scale);
            _mons.push_back(m);
        }
    }
    return ;
}

