#include "GDIWindowProcessor.h"
#include "GDIHelper.h"
#include "StringTools.h"

#include <dwmapi.h>
#include <array>

#include <dwmapi.h>
#include <array>
#include <algorithm>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <iostream>

struct srch {
    WindowVector * win;
    Pid pid;
    bool all;
};

typedef struct Window
{
public:
    Window(nullptr_t) {}
    Window(HWND hwnd, std::wstring const& title, std::wstring& className)
    {
        m_hwnd = hwnd;
        m_title = title;
        m_className = className;
    }

    HWND Hwnd() const noexcept { return m_hwnd; }
    std::wstring Title() const noexcept { return m_title; }
    std::wstring ClassName() const noexcept { return m_className; }

private:
    HWND m_hwnd;
    std::wstring m_title;
    std::wstring m_className;
} Window;

std::wstring GetClassName(HWND hwnd)
{
	std::array<WCHAR, 1024> className;

    ::GetClassNameW(hwnd, className.data(), (int)className.size());

    std::wstring title(className.data());
    return title;
}

std::wstring GetWindowText(HWND hwnd)
{
	std::array<WCHAR, 1024> windowText;

    ::GetWindowTextW(hwnd, windowText.data(), (int)windowText.size());

    std::wstring title(windowText.data());
    return title;
}

bool IsAltTabWindow(Window const& window)
{
    HWND hwnd = window.Hwnd();
    HWND shellWindow = GetShellWindow();

    auto title = window.Title();
    auto className = window.ClassName();

    if (hwnd == shellWindow)
    {
        return false;
    }

    if (title.length() == 0)
    {
        return false;
    }

    if (!IsWindowVisible(hwnd))
    {
        return false;
    }

    if (GetAncestor(hwnd, GA_ROOT) != hwnd)
    {
        return false;
    }

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (!((style & WS_DISABLED) != WS_DISABLED))
    {
        return false;
    }

    DWORD cloaked = FALSE;
    HRESULT hrTemp = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (SUCCEEDED(hrTemp) &&
        cloaked == DWM_CLOAKED_SHELL)
    {
        return false;
    }

    return true;
}

static BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
{
    srch *s = (srch *)lParam;
    DWORD pid;
    wchar_t Name[1024] = { '\0' };

    auto class_name = GetClassName(hwnd);
    auto title = GetWindowText(hwnd);
    auto window = Window(hwnd, title, class_name);

    /* skip not valid window */
    if (!IsAltTabWindow(window))
    {
        return TRUE;
    }


    /* get PID first */
    GetWindowThreadProcessId(hwnd, &pid);
    /* check if pid is desired or not */
    if(s->pid != -1 && s->pid != pid) return true;

    if (pid != GetCurrentProcessId()) {
        auto textlen = GetWindowTextW(hwnd, Name, sizeof(Name));
    }

    /* window name only return the windows with name */
    if(!s->all && wcslen (Name)==0) return true;

    auto windowrect = GetWindowRect(hwnd);

    std::transform(std::begin(Name), std::end(Name), std::begin(Name), ::tolower);

    CapPoint offset(windowrect.ClientRect.left, windowrect.ClientRect.top);
    CapPoint size (windowrect.ClientRect.right - windowrect.ClientRect.left,
                    windowrect.ClientRect.bottom - windowrect.ClientRect.top);

    CapWindow w(reinterpret_cast<size_t>(hwnd), offset, size, utf16_to_utf8(Name), pid);

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
