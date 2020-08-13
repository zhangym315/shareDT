#include "GDIWindowProcessor.h"
#include "GDIHelper.h"

#include <algorithm>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <iostream>

struct srch {
    WindowVector * win;
    Pid pid;
    bool all;
};

static BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    srch *s = (srch *)lParam;
    DWORD pid;
    char Name[1024] = { '\0' };

    /* get PID first */
    GetWindowThreadProcessId(hwnd, &pid);
    /* check if pid is desired or not */
    if(s->pid != -1 && s->pid != pid) return true;

    if (pid != GetCurrentProcessId()) {
        auto textlen = GetWindowTextA(hwnd, Name, sizeof(Name));
    }

    /* window name only return the windows with name */
    if(!s->all && strlen(Name)==0) return true;

    auto windowrect = GetWindowRect(hwnd);

    std::transform(std::begin(Name), std::end(Name), std::begin(Name), ::tolower);

    CapPoint offset(windowrect.ClientRect.left, windowrect.ClientRect.top);
    CapPoint size (windowrect.ClientRect.right - windowrect.ClientRect.left,
                    windowrect.ClientRect.bottom - windowrect.ClientRect.top);

    CapWindow w(reinterpret_cast<size_t>(hwnd), offset, size, Name, pid);

    s->win->push_back(w);
    return TRUE;
}

void WindowVectorProvider::CapGetWindows()
{
    srch s;
    s.win = &_wins;
    s.pid = _pid;
    s.all = _getAll;
    EnumWindows(EnumWindowsProc, (LPARAM)&s);
    return ;
}
