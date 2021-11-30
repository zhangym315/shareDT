#import "KeyboardEvents.h"
#include <Carbon/Carbon.h>

#include "InputInterface.h"
#include "Logger.h"

@implementation KeyboardEvents

+(void) pressKeyboardEvent:(CGKeyCode)keyCode isDown:(bool)down {
    CGEventRef press = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, down);
    CGEventPost(kCGHIDEventTap, press);
    CFRelease(press);
}

@end

typedef std::map<String, CGKeyCode> KeyCode;

#define KVK_ANSI(x) {#x, kVK_ANSI_##x },
#define KVK(x)      {#x, kVK_##x },

static KeyCode IOSKEYCODE = {
        KVK_ANSI(A)
        KVK_ANSI(B)
        KVK_ANSI(C)
        KVK_ANSI(D)
        KVK_ANSI(E)
        KVK_ANSI(F)
        KVK_ANSI(G)
        KVK_ANSI(H)
        KVK_ANSI(I)
        KVK_ANSI(J)
        KVK_ANSI(K)
        KVK_ANSI(L)
        KVK_ANSI(M)
        KVK_ANSI(N)
        KVK_ANSI(O)
        KVK_ANSI(P)
        KVK_ANSI(Q)
        KVK_ANSI(R)
        KVK_ANSI(S)
        KVK_ANSI(T)
        KVK_ANSI(U)
        KVK_ANSI(V)
        KVK_ANSI(W)
        KVK_ANSI(X)
        KVK_ANSI(Y)
        KVK_ANSI(Z)

        KVK_ANSI(0)
        KVK_ANSI(1)
        KVK_ANSI(2)
        KVK_ANSI(3)
        KVK_ANSI(4)
        KVK_ANSI(5)
        KVK_ANSI(6)
        KVK_ANSI(7)
        KVK_ANSI(8)
        KVK_ANSI(9)

        KVK(F1)
        KVK(F2)
        KVK(F3)
        KVK(F4)
        KVK(F5)
        KVK(F6)
        KVK(F7)
        KVK(F8)
        KVK(F9)
        KVK(F10)
        KVK(F11)
        KVK(F12)
        KVK(F13)
        KVK(F14)
        KVK(F15)
        KVK(F16)
        KVK(F17)
        KVK(F18)
        KVK(F19)
        KVK(F20)

        {"Minus",        kVK_ANSI_Minus},
        {"Equal",        kVK_ANSI_Equal},
        {"RightBracket", kVK_ANSI_RightBracket},
        {"LeftBracket",  kVK_ANSI_LeftBracket},
        {"Quote",        kVK_ANSI_Quote},
        {"Semicolon",    kVK_ANSI_Semicolon},

        {"Return",       kVK_Return},
        {"Tab",          kVK_Tab},
        {"Space",        kVK_Space},
        {"Delete",       kVK_Delete},
        {"Escape",       kVK_Escape},
        {"Command",      kVK_Command},
        {"Shift",        kVK_Shift},
        {"CapsLock",     kVK_CapsLock},
        {"Option",       kVK_Option},
        {"Control",      kVK_Control},
        {"RightCommand", kVK_RightCommand},
        {"RightShift",   kVK_RightShift},

        {"Home",          kVK_Home},
        {"PageUp",        kVK_PageUp},
        {"PageDown",      kVK_PageDown},
        {"ForwardDelete", kVK_ForwardDelete},
        {"End",           kVK_End},
        {"LeftArrow",     kVK_LeftArrow},
        {"RightArrow",    kVK_RightArrow},
        {"DownArrow",     kVK_DownArrow},
        {"UpArrow",       kVK_UpArrow},

        {"Backslash"     , kVK_ANSI_Backslash      },
        {"Comma"         , kVK_ANSI_Comma          },
        {"Slash"         , kVK_ANSI_Slash          },
        {"Period"        , kVK_ANSI_Period         },
        {"Grave"         , kVK_ANSI_Grave          },
        {"KeypadDecimal" , kVK_ANSI_KeypadDecimal  },
        {"KeypadMultiply", kVK_ANSI_KeypadMultiply },
        {"KeypadPlus"    , kVK_ANSI_KeypadPlus     },
        {"KeypadClear"   , kVK_ANSI_KeypadClear    },
        {"KeypadDivide"  , kVK_ANSI_KeypadDivide   },
        {"KeypadEnter"   , kVK_ANSI_KeypadEnter    },
        {"KeypadMinus"   , kVK_ANSI_KeypadMinus    },
        {"KeypadEquals"  , kVK_ANSI_KeypadEquals   },
        {"Keypad0"       , kVK_ANSI_Keypad0        },
        {"Keypad1"       , kVK_ANSI_Keypad1        },
        {"Keypad2"       , kVK_ANSI_Keypad2        },
        {"Keypad3"       , kVK_ANSI_Keypad3        },
        {"Keypad4"       , kVK_ANSI_Keypad4        },
        {"Keypad5"       , kVK_ANSI_Keypad5        },
        {"Keypad6"       , kVK_ANSI_Keypad6        },
        {"Keypad7"       , kVK_ANSI_Keypad7        },
        {"Keypad8"       , kVK_ANSI_Keypad8        },
        {"Keypad9"       , kVK_ANSI_Keypad9        },

        {"Help"          , kVK_Help           },
        {"RightOption"   , kVK_RightOption    },
        {"RightControl"  , kVK_RightControl   },
        {"Function"      , kVK_Function       },
        {"VolumeUp"      , kVK_VolumeUp       },
        {"VolumeDown"    , kVK_VolumeDown     },
        {"Underscore"    , kVK_JIS_Underscore },
        {"Mute"          , kVK_Mute           }
};

void InputMousePlatform::keyboardClick(int isDown, const String & k)
{
    if (k.empty() || (IOSKEYCODE.find(k)==IOSKEYCODE.end())) return;

    CGKeyCode code = IOSKEYCODE[k];

    [KeyboardEvents pressKeyboardEvent:code isDown:(isDown)];
}
