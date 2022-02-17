#include "Logger.h"
#include "InputInterface.h"
#include "CaptureInfo.h"

#include <WinUser.h>

static void initMouse(INPUT* buffer)
{
    buffer->type = INPUT_MOUSE;
    buffer->mi.mouseData = 0;
    buffer->mi.dwFlags = MOUSEEVENTF_ABSOLUTE;
    buffer->mi.time = 0;
    buffer->mi.dwExtraInfo = 0;
}

/* Interface for Windows */
void InputMousePlatform::mouseClickAtCordinate(Cordinate c, MouseButton b, int count)
{
    INPUT inputMouse;

    initMouse(&inputMouse);

    // first handle wheel action
    if ((b&~MouseButtonMask) == WheeleMoved) {
        inputMouse.mi.dwFlags = MOUSEEVENTF_WHEEL;
        inputMouse.mi.mouseData = c._x;

        SendInput(1, &inputMouse, sizeof(INPUT));
        return;
    }

    inputMouse.type = INPUT_MOUSE;
    inputMouse.mi.dx = c._x * (0xFFFF / CaptureInfo::instance()->getCapMonitor().getWidth());
    inputMouse.mi.dy = c._y * (0xFFFF / CaptureInfo::instance()->getCapMonitor().getHeight());

    switch (b & MouseButtonMask)
    {
        case LeftButton:
            inputMouse.mi.dwFlags |=  ((b & ~MouseButtonMask) == ButtonDown) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
            break;
        case RightButton:
            inputMouse.mi.dwFlags |=  ((b & ~MouseButtonMask) == ButtonDown) ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
            break;
        case MiddleButton:
            inputMouse.mi.dwFlags |=  ((b & ~MouseButtonMask) == ButtonDown) ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
            break;
        case NoButton:
            inputMouse.mi.dwFlags |= MOUSEEVENTF_MOVE;
            break;  
        // TODO, for X botton and others, donot handle
        default:
            LOGGER.warn() << "Reveived out of range botton value button=" << b << " inputMouse.mi.dx=" << inputMouse.mi.dx << " inputMouse.mi.dy=" << inputMouse.mi.dy;
            return;
    }

    SendInput(1, &inputMouse, sizeof(INPUT));
}
