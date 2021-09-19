#ifndef SHAREDT_INPUTINTERFACE_H
#define SHAREDT_INPUTINTERFACE_H
#include <rfb/rfb.h>

typedef enum MouseButton {
    NoButton         = 0x00000000,
    LeftButton       = 0x00000001,
    RightButton      = 0x00000002,
    MiddleButton     = 0x00000004,
    ExtraButton1     = 0x00000008,
    ExtraButton2     = 0x00000010,
    ExtraButton3     = 0x00000020,
    ExtraButton4     = 0x00000040,
    ExtraButton5     = 0x00000080,
    ExtraButton6     = 0x00000100,
    ExtraButton7     = 0x00000200,
    ExtraButton8     = 0x00000400,
    ExtraButton9     = 0x00000800,
    ExtraButton10    = 0x00001000,
    ExtraButton11    = 0x00002000,
    ExtraButton12    = 0x00004000,
    ExtraButton13    = 0x00008000,
    ExtraButton14    = 0x00010000,
    ExtraButton15    = 0x00020000,
    ExtraButton16    = 0x00040000,
    ExtraButton17    = 0x00080000,
    ExtraButton18    = 0x00100000,
    ExtraButton19    = 0x00200000,
    ExtraButton20    = 0x00400000,
    ExtraButton21    = 0x00800000,
    ExtraButton22    = 0x01000000,
    ExtraButton23    = 0x02000000,
    ExtraButton24    = 0x04000000,
    AllButtons       = 0x07ffffff,
    MaxMouseButton   = ExtraButton24,
    MouseButtonMask  = 0x0fffffff,
    ButtonDown       = 0x10000000,
    ButtonUp         = 0x20000000,
    WheeleMoved      = 0x40000000,
    MousePressMask   = 0xf0000000
} MouseButton;

typedef enum ScrollDirection {
    ScrollNoDirection = 0,
    ScrollUp          = 1,
    ScroolDown        = 2,
    ScrollRight       = 3,
    ScroolLeft        = 4
} ScrollDirection;

typedef struct Cordinate {
    Cordinate() : _x(0), _y(0) { }
    Cordinate(int x, int y) : _x(x), _y(y) { }
    int _x, _y;
} Cordinate;


extern void ptrServerMouseEvent(int buttonMask, int x, int y, rfbClientPtr cl);

class InputMousePlatform {
public:
    static void mouseClickAtCordinate(Cordinate c, MouseButton b, int clickCount);

};
#endif //SHAREDT_INPUTINTERFACE_H
