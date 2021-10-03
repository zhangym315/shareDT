#include "Logger.h"
#include "InputInterface.h"

#include <WinUser.h>

/* Interface for Windows */
void InputMousePlatform::mouseClickAtCordinate(Cordinate c, MouseButton b, int count)
{
    INPUT inputMouse;

    // first handle wheel action
    if ((b&~MouseButtonMask) == WheeleMoved) {
        inputMouse.mi.dwFlags = MOUSEEVENTF_WHEEL;
        inputMouse.mi.mouseData = c._x;

        SendInput(1, &inputMouse, sizeof(INPUT));
        return;
    }

    inputMouse.type = INPUT_MOUSE;
    inputMouse.mi.dx = c._x;
    inputMouse.mi.dy = c._y;

    switch (b & MouseButtonMask)
    {
        case LeftButton:
            inputMouse.mi.dwFlags =  ((b & ~MouseButtonMask) == ButtonDown) ? MOUSEEVENTF_LEFTDOWN :
                                    MOUSEEVENTF_LEFTUP;
            break;
        case RightButton:
            inputMouse.mi.dwFlags =  ((b & ~MouseButtonMask) == ButtonDown) ? MOUSEEVENTF_RIGHTDOWN :
                                    MOUSEEVENTF_RIGHTUP;
            break;
        case MiddleButton:
            inputMouse.mi.dwFlags =  ((b & ~MouseButtonMask) == ButtonDown) ? MOUSEEVENTF_MIDDLEDOWN :
                                    MOUSEEVENTF_MIDDLEUP;
            break;
        case NoButton:
            break;
        // TODO, for X botton and others, donot handle
        default:
            LOGGER.warn() << "Reveived outof range botton value botton=" << b;
            return;
    }

    inputMouse.mi.dwFlags |= MOUSEEVENTF_MOVE;
    SendInput(1, &inputMouse, sizeof(INPUT));
}
