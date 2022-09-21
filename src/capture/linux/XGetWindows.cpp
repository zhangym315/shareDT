#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include "WindowProcessor.h"

class UniqueTextProperty {
  public:
    UniqueTextProperty()
    {
        p.value = nullptr;
    }

    UniqueTextProperty(const UniqueTextProperty&) = delete;
    UniqueTextProperty& operator=(const UniqueTextProperty&) = delete;

    UniqueTextProperty(UniqueTextProperty&& other):
        p{other.p}
    {
        other.p = XTextProperty{};
    }

    UniqueTextProperty& operator=(UniqueTextProperty&& other)
    {
        swap(*this, other);
        return *this;
    }

    friend void swap(UniqueTextProperty& lhs, UniqueTextProperty& rhs) {
        using std::swap;
        swap(lhs.p, rhs.p);
    }

    ~UniqueTextProperty()
    {
        if (p.value) {
                XFree(p.value);
            }
    }

    auto& get() {
        return p;
    }

  private:
    XTextProperty p;
};

auto GetWMName(Display* display, Window window)
{
    auto x = UniqueTextProperty{};
    XGetWMName(display, window, &x.get());
    return x;
}

auto TextPropertyToStrings(
    Display* dpy,
    const XTextProperty& prop
)
{
    char **list;
    auto n_strings = 0;
    auto result = std::vector<std::string>{};

    auto status = XmbTextPropertyToTextList(
        dpy,
        &prop,
        &list,
        &n_strings
    );

    if (status < Success or not n_strings or not *list) {
            return result;
        }

    for (auto i = 0; i < n_strings; ++i) {
            result.emplace_back(list[i]);
        }

    XFreeStringList(list);

    return result;
}

void AddWindow(Display* display, XID& window, WindowVector & wnd, bool all)
{
    using namespace std::string_literals;

    auto wm_name = GetWMName(display, window);
    auto candidates = TextPropertyToStrings(display, wm_name.get());

    XWindowAttributes wndattr;
    XGetWindowAttributes(display, window, &wndattr);

    CapPoint offset( wndattr.x, wndattr.y);
    CapPoint size (wndattr.width, wndattr.height);

    auto name = candidates.empty() ? ""s : std::move(candidates.front());

    if(!all && name.length()==0 ) return;

    /* for linux, there is not support to fetch pid from window handler */
    CapWindow w(static_cast<size_t>(window), offset, size, name, 0);
    wnd.push_back(w);
}

static Display * display = nullptr;

void WindowVectorProvider::CapGetWindows()
{
    if (display == nullptr)
        display = XOpenDisplay(NULL);
    if (display == NULL) return;

    Atom a = XInternAtom(display, "_NET_CLIENT_LIST", true);
    Atom actualType;
    int format;
    unsigned long numItems, bytesAfter;
    unsigned char* data = 0;
    int status = XGetWindowProperty(display,
                                    XDefaultRootWindow(display),
                                    a,
                                    0L,
                                    (~0L),
                                    false,
                                    AnyPropertyType,
                                    &actualType,
                                    &format,
                                    &numItems,
                                    &bytesAfter,
                                    &data);

    if(status >= Success && numItems) {
        auto array = (XID*)data;
        for(decltype(numItems) k = 0; k < numItems; k++) {
            auto w = array[k];
            AddWindow(display, w, _wins, _getAll);
        }
        XFree(data);
    }
    //XCloseDisplay(display);
    return ;
}
