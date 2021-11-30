#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <rfb/keysym.h>
#include <map>

#include "InputInterface.h"
#include "Logger.h"

static Display * display = nullptr;

typedef std::map<String, KeySym> KeyCodeLinux;

#define XK(x) {#x, XK_##x },

/*
 * Linux key code
 * Defined in rfb/keysym.h
 */
static KeyCodeLinux LINUXKEYCODE = {
        XK(A)
        XK(B)
        XK(C)
        XK(D)
        XK(E)
        XK(F)
        XK(G)
        XK(H)
        XK(I)
        XK(J)
        XK(K)
        XK(L)
        XK(M)
        XK(N)
        XK(O)
        XK(P)
        XK(Q)
        XK(R)
        XK(S)
        XK(T)
        XK(U)
        XK(V)
        XK(W)
        XK(X)
        XK(Y)
        XK(Z)

        XK(0)
        XK(1)
        XK(2)
        XK(3)
        XK(4)
        XK(5)
        XK(6)
        XK(7)
        XK(8)
        XK(9)

        XK(F1)
        XK(F2)
        XK(F3)
        XK(F4)
        XK(F5)
        XK(F6)
        XK(F7)
        XK(F8)
        XK(F9)
        XK(F10)
        XK(F11)
        XK(F13)
        XK(F13)
        XK(F14)
        XK(F15)
        XK(F16)
        XK(F17)
        XK(F18)
        XK(F19)
        XK(F20)

        XK(L1)
        XK(L2)
        XK(L3)
        XK(L4)
        XK(L5)
        XK(L6)
        XK(L7)
        XK(L8)
        XK(L9)

        {"Delete",        XK_BackSpace  },
        {"Tab",           XK_Tab        },
        {"Linefeed",      XK_Linefeed   },
        {"Clear",         XK_Clear      },
        {"Return",        XK_Return     },
        {"Pause",         XK_Pause      },
        {"ScrollLck",     XK_Scroll_Lock},
        {"SysReq",        XK_Sys_Req	},
        {"Escape",        XK_Escape     },
        {"ForwardDelete", XK_Delete     },

        {"Shift",         XK_Shift_L    },
        {"RightShift",    XK_Shift_R    },
        {"Control",       XK_Control_L  },
        {"RightControl",  XK_Control_R  },
        {"Option",        XK_Alt_L      },   // left ALT
        {"CapsLock",      XK_Caps_Lock  },
        {"Command",       XK_Meta_L     },

        {"Space",        XK_space       },
        {"Exclam",       XK_exclam      },
        {"QuotedBl",     XK_quotedbl    },
        {"NumberSign",   XK_numbersign  },
        {"Dollar",       XK_dollar      },
        {"Percent",      XK_percent     },
        {"Ampersand",    XK_ampersand   },
        {"Quote",        XK_apostrophe  },
        {"QuoteRight",   XK_quoteright  },
        {"ParenLeft",    XK_parenleft   },
        {"ParenRight",   XK_parenright  },
        {"Asterisk",     XK_asterisk    },
        {"KeypadPlus",   XK_plus        },
        {"Comma",        XK_comma       },
        {"Minus",        XK_minus       },
        {"Period",       XK_period      },
        {"Slash",        XK_slash       },
        {"VerticalBar",  XK_bar         },

        {"Colon",        XK_colon       },
        {"Semicolon",    XK_semicolon   },
        {"Less",         XK_less        },
        {"Equal",        XK_equal       },
        {"Greater",      XK_greater     },
        {"Question",     XK_question    },
        {"At",           XK_at          },

        {"LeftBracket",  XK_bracketleft },   // [
        {"Backslash",    XK_backslash   },
        {"RightBracket", XK_bracketright},   // ]
        {"Asciicircum",  XK_asciicircum },
        {"UnderScore",   XK_underscore  },
        {"Grave",        XK_grave       },

        {"Home",         XK_Home        },
        {"LeftArrow",    XK_Left        },
        {"UpArrow",      XK_Up          },
        {"RightArrow",   XK_Right       },
        {"DownArrow",    XK_Down        },
        {"PageUp",       XK_Page_Up     },
        {"PageDown",     XK_Page_Down   },
        {"End",          XK_End         },
        {"Pior",         XK_Prior       },  // Todo no implement start
        {"Next",         XK_Next        },
        {"Begin",        XK_Begin       }   // Todo no implement end
};

void InputMousePlatform::keyboardClick(int isDown, const String  & key)
{
    if (key.empty() || LINUXKEYCODE.find(key)==LINUXKEYCODE.end()) return;

    if (!display) display = XOpenDisplay(0);

    KeyCode code;

    code = XKeysymToKeycode(display, LINUXKEYCODE[key]);
    if (code == 0)
        return;

    XTestFakeKeyEvent(display, code, isDown,  CurrentTime);
    XFlush(display);
}
