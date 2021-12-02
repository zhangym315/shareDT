#include "InputInterface.h"
#include <WinUser.h>

bool sendKey(WORD code, bool isDown)
{
    INPUT input;
    ZeroMemory(&input, sizeof(input));

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = code;

    if (!isDown)     input.ki.dwFlags = KEYEVENTF_KEYUP;

    if (SendInput(1, &input, sizeof(INPUT)) != 1) {
        return false;
    }

    return true;
}

typedef std::map<String, WORD> KeyCode;

/*
 * Windows key code
 * https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
 */
static KeyCode WINKEYCODE = {
        {"A", 0x41},
        {"B", 0x42},
        {"C", 0x43},
        {"D", 0x44},
        {"E", 0x45},
        {"F", 0x46},
        {"G", 0x47},
        {"H", 0x48},
        {"I", 0x49},
        {"J", 0x4A},
        {"K", 0x4B},
        {"L", 0x4C},
        {"M", 0x4D},
        {"N", 0x4E},
        {"O", 0x4F},
        {"P", 0x50},
        {"Q", 0x51},
        {"R", 0x52},
        {"S", 0x53},
        {"T", 0x54},
        {"U", 0x55},
        {"V", 0x56},
        {"W", 0x57},
        {"X", 0x58},
        {"Y", 0x59},
        {"Z", 0x5A},

        {"0", 0x30},
        {"1", 0x31},
        {"2", 0x32},
        {"3", 0x33},
        {"4", 0x34},
        {"5", 0x35},
        {"6", 0x36},
        {"7", 0x37},
        {"8", 0x38},
        {"9", 0x39},

        {"Minus",        VK_OEM_MINUS},
        {"Equal",        VK_OEM_PLUS},
        {"RightBracket", VK_OEM_6},
        {"LeftBracket",  VK_OEM_4},
        {"Quote",        VK_OEM_7},
        {"Semicolon",    VK_OEM_1},

        {"Return",       VK_RETURN   },
        {"Tab",          VK_TAB      },
        {"Space",        VK_SPACE    },
        {"Delete",       VK_BACK     },
        {"Escape",       VK_ESCAPE   },
        {"Command",      VK_LMENU    },   // left ALT
        {"Shift",        VK_LSHIFT   },
        {"CapsLock",     VK_CAPITAL  },
        {"Option",       VK_MENU     },
        {"Control",      VK_CONTROL  },
        {"RightCommand", VK_RWIN     },
        {"RightShift",   VK_RSHIFT   },

        {"Home",          VK_HOME    },
        {"PageUp",        VK_PRIOR   },
        {"PageDown",      VK_NEXT    },
        {"ForwardDelete", VK_DELETE  },
        {"End",           VK_END     },
        {"LeftArrow",     VK_LEFT    },
        {"RightArrow",    VK_RIGHT   },
        {"DownArrow",     VK_DOWN    },
        {"UpArrow",       VK_UP      },

        {"F1",  VK_F1},
        {"F2",  VK_F2},
        {"F3",  VK_F3},
        {"F4",  VK_F4},
        {"F5",  VK_F5},
        {"F6",  VK_F6},
        {"F7",  VK_F7},
        {"F8",  VK_F8},
        {"F9",  VK_F9},
        {"F10", VK_F10},
        {"F11", VK_F11},
        {"F12", VK_F12},
        {"F13", VK_F13},
        {"F14", VK_F14},
        {"F15", VK_F15},
        {"F16", VK_F16},
        {"F17", VK_F17},
        {"F18", VK_F18},
        {"F19", VK_F19},
        {"F20", VK_F20},

        {"Backslash"     , VK_OEM_5       },
        {"Comma"         , VK_OEM_COMMA   },
        {"Slash"         , VK_OEM_2       },
        {"Period"        , VK_OEM_PERIOD  },
        {"Sleep"         , VK_SLEEP       },
        {"Grave"         , VK_OEM_3       },

        {"Tilde"         , VK_OEM_3       },
        {"Exclam"        , 0x31           }, // shift + 1
        {"At"            , 0x32           },
        {"NumberSign"    , 0x33           },
        {"Dollar"        , 0x34           },
        {"Percent"       , 0x35           },
        {"Asciicircum"   , 0x36           },
        {"Ampersand"     , 0x37           },   // Shift+7
        {"Asterisk"      , 0x38           },   // Shift+8
        {"ParenLeft"     , 0x39           },   // Shift+9
        {"ParenRight"    , 0x30           },   // Shift+0
        {"Plus"          , VK_OEM_PLUS    },   // Shift+Equal (=)
        {"UnderScore"    , VK_OEM_MINUS   },   // Shift+Minus (-)

        {"RightCurlyBkt" , VK_OEM_6       },   // Shift+BaskSlash
        {"LeftCurlyBkt"  , VK_OEM_4       },   // Shift+BaskSlash
        {"VerticalBar"   , VK_OEM_5       },   // Shift+BaskSlash
        {"QuotedBl"      , VK_OEM_7       },   // Shift+Quote
        {"Colon"         , VK_OEM_1       },   // Shift+Semicolon
        {"Less"          , VK_OEM_COMMA   },   // Shift+Comma (,)
        {"Greater"       , VK_OEM_PERIOD  },   // Shift+Period(.)
        {"Question"      , VK_OEM_2       },   // Shift+Slash(/)

        {"KeypadDecimal" , VK_DECIMAL     },
        {"KeypadMultiply", VK_MULTIPLY    },
        {"KeypadPlus"    , VK_ADD         },
//        {"KeypadClear"   , KeypadClear  },
        {"KeypadDivide"  , VK_SUBTRACT    },
//        {"KeypadEnter"   , KeypadEnter  },
        {"KeypadMinus"   , VK_SUBTRACT    },
        {"KeypadEquals"  , VK_SUBTRACT    },
        {"Keypad0"       , VK_NUMPAD0     },
        {"Keypad1"       , VK_NUMPAD1     },
        {"Keypad2"       , VK_NUMPAD2     },
        {"Keypad3"       , VK_NUMPAD3     },
        {"Keypad4"       , VK_NUMPAD4     },
        {"Keypad5"       , VK_NUMPAD5     },
        {"Keypad6"       , VK_NUMPAD6     },
        {"Keypad7"       , VK_NUMPAD7     },
        {"Keypad8"       , VK_NUMPAD8     },
        {"Keypad9"       , VK_NUMPAD9     },
        {"KeypadNumLk"   , VK_NUMLOCK     },

/*        {"RightOption"   , RightOption  },
        {"RightControl"  , RightControl   },
        {"Function"      , Function       },  
 */

        /* Windows specific key */
        {"KeyPad_Separator", VK_SEPARATOR },
        {"Select",        VK_SELECT       },
        {"Print",         VK_PRINT        },
        {"Snapshot",      VK_SNAPSHOT     },
        {"Help",          VK_HELP	      },
        {"Scroll",        VK_SCROLL       },

        {"VolumeUp"      , VK_VOLUME_UP   },
        {"VolumeDown"    , VK_VOLUME_DOWN },
        {"Mute"          , VK_VOLUME_MUTE }
};

void InputMousePlatform::keyboardClick(int isDown, const String  & key)
{
    if (key.empty() || WINKEYCODE.find(key)==WINKEYCODE.end()) {
        LOGGER.info() << "Can't get code for key=" << key ;
        return;
    }

    sendKey(WINKEYCODE[key], isDown);
}

