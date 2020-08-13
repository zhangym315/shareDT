#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <dlfcn.h>
#include "WindowProcessor.h"

void MonitorVectorProvider::CapGetMonitors ()
{
    Display* display = XOpenDisplay(NULL);
    if(display==NULL){
        _mons.clear();
        return;
    }
    int nmonitors = 0;
    XineramaScreenInfo* screen = XineramaQueryScreens(display, &nmonitors);
    if(screen==NULL){
        XCloseDisplay(display);
        _mons.clear();
        return;
    }

    for(auto i = 0; i < nmonitors; i++) {
        CapMonitor m(_mons.size(), screen[i].screen_number, screen[i].width, screen[i].height, screen[i].x_org, screen[i].y_org, 1.0f);
        _mons.push_back(m);
    }
    XFree(screen);
    XCloseDisplay(display);
    return ;
}
