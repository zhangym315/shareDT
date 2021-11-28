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

KeyCode IOSKEYCODE = {
        {"A", kVK_ANSI_A},
        {"B", kVK_ANSI_B},
        {"C", kVK_ANSI_C},
        {"D", kVK_ANSI_D},
        {"E", kVK_ANSI_E},
        {"F", kVK_ANSI_F},
        {"G", kVK_ANSI_G},
        {"H", kVK_ANSI_H},
        {"I", kVK_ANSI_I},
        {"J", kVK_ANSI_J},
        {"K", kVK_ANSI_K},
        {"L", kVK_ANSI_L},
        {"M", kVK_ANSI_M},
        {"N", kVK_ANSI_N},
        {"O", kVK_ANSI_O},
        {"P", kVK_ANSI_P},
        {"Q", kVK_ANSI_Q},
        {"R", kVK_ANSI_R},
        {"S", kVK_ANSI_S},
        {"T", kVK_ANSI_T},
        {"U", kVK_ANSI_U},
        {"V", kVK_ANSI_V},
        {"W", kVK_ANSI_W},
        {"X", kVK_ANSI_X},
        {"Y", kVK_ANSI_Y},
        {"Z", kVK_ANSI_Z},

        {"0",            kVK_ANSI_0},
        {"1",            kVK_ANSI_1},
        {"2",            kVK_ANSI_2},
        {"3",            kVK_ANSI_3},
        {"4",            kVK_ANSI_4},
        {"5",            kVK_ANSI_5},
        {"6",            kVK_ANSI_6},
        {"7",            kVK_ANSI_7},
        {"8",            kVK_ANSI_8},
        {"9",            kVK_ANSI_9},

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

        {"F1",  kVK_F1},
        {"F2",  kVK_F2},
        {"F3",  kVK_F3},
        {"F4",  kVK_F4},
        {"F5",  kVK_F5},
        {"F6",  kVK_F6},
        {"F7",  kVK_F7},
        {"F8",  kVK_F8},
        {"F9",  kVK_F9},
        {"F10", kVK_F10},
        {"F11", kVK_F11},
        {"F12", kVK_F12},
        {"F13", kVK_F13},
        {"F14", kVK_F14},
        {"F15", kVK_F15},
        {"F16", kVK_F16},
        {"F17", kVK_F17},
        {"F18", kVK_F18},
        {"F19", kVK_F19},
        {"F20", kVK_F20},

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
        {"Mute"          , kVK_Mute           }
};

void InputMousePlatform::keyboardClick(int isDown, const String & k)
{
    if (k.empty() || (IOSKEYCODE.find(k)==IOSKEYCODE.end())) return;

    CGKeyCode code = IOSKEYCODE[k];

    [KeyboardEvents pressKeyboardEvent:code isDown:(isDown)];
}
