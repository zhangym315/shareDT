#include "InputInterface.h"
#include "Logger.h"

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

static Display * display = nullptr;

void InputMousePlatform::mouseClickAtCordinate(Cordinate c, MouseButton b, int count)
{
    if (!display) display = XOpenDisplay(0);
    
    // handle wheele move first
    if ((b&~MouseButtonMask) == WheeleMoved) {
        int upOrdown = (c._x > 0) ? Button4 : Button5;
        XTestFakeButtonEvent(display, upOrdown, true, CurrentTime);
        XSync(display, false);
        XTestFakeButtonEvent(display, upOrdown, false, CurrentTime);
        XFlush(display);
        return;
    }

    switch (b & MouseButtonMask)
    {
        case LeftButton:
            XTestFakeButtonEvent(display, Button1, 
                                (((b & ~MouseButtonMask) == ButtonDown) ? true : false), /* is_press */
                                CurrentTime);
            XFlush(display);

            break;
        case RightButton:
            XTestFakeButtonEvent(display, Button2,
                                (((b & ~MouseButtonMask) == ButtonDown) ? true : false), /* is_press */
                                CurrentTime);
            XFlush(display);

            break;
        case MiddleButton:
            XTestFakeButtonEvent(display, Button3,
                                (((b & ~MouseButtonMask) == ButtonDown) ? true : false), /* is_press */
                                CurrentTime);
            XFlush(display);

            break;
        case NoButton:
            XTestFakeMotionEvent(display, -1, c._x, c._y, CurrentTime);
            XFlush(display);
            break;
        default:
            LOGGER.warn() << "Invalid button received button=" << b;
            break;
    }
}