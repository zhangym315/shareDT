#include "MouseEvents.h"
#include "InputInterface.h"

#include <WinUser.h>

static INPUT inputMouse;

void MouseInputWin::leftMouseDownAt(int x, int y) 
{

}

void MouseInputWin::rightMouseDownAt(int x, int y)
{

}

void MouseInputWin::middleMouseDownAt(int x, int y)
{

}

void MouseInputWin::leftMouseUpAt(int x, int y)
{

}

void MouseInputWin::rightMouseUpAt(int x, int y)
{

}

void MouseInputWin::middleMouseUpAt(int x, int y)
{

}

void MouseInputWin::scrollDown(int lines)
{

}

void MouseInputWin::scrollUp(int lines)
{

}

void MouseInputWin::scrollLeft(int lines)
{

}

void MouseInputWin::scrollRight(int lines)
{

}

/* Interface for Windows */
void InputMousePlatform::mouseClickAtCordinate(Cordinate c, MouseButton b, int count)
{
    inputMouse.type = INPUT_MOUSE;
    inputMouse.mi.dx = c._x;
    inputMouse.mi.dy = c._y;

    inputMouse.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    SendInput(1, &inputMouse, sizeof(INPUT));
}
