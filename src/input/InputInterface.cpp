#include <fstream>

#include "InputInterface.h"
#include "Logger.h"
#include "Path.h"

const static std::string CODEKEY_PATH = "/bin/KeyCode.txt";
const static std::string CODEKEY_EMPTY = "";

void ptrServerMouseEvent(int buttonMask, int x, int y, rfbClientPtr cl)
{
    rfbClientIteratorPtr iterator;
    rfbClientPtr other_client;
    rfbScreenInfoPtr s = cl->screen;

    InputMousePlatform::mouseClickAtCordinate(Cordinate(x, y), (MouseButton) buttonMask, 1);

    if (x != s->cursorX || y != s->cursorY) {
        LOCK(s->cursorMutex);
        s->cursorX = x;
        s->cursorY = y;
        UNLOCK(s->cursorMutex);

        /* The cursor was moved by this client, so don't send CursorPos. */
        if (cl->enableCursorPosUpdates)
            cl->cursorWasMoved = FALSE;

        /* But inform all remaining clients about this cursor movement. */
        iterator = rfbGetClientIterator(s);
        while ((other_client = rfbClientIteratorNext(iterator)) != NULL) {
            if (other_client != cl && other_client->enableCursorPosUpdates) {
                other_client->cursorWasMoved = TRUE;
            }
        }
        rfbReleaseClientIterator(iterator);
    }
}

void kbdServerKeyEvent(rfbBool down, rfbKeySym keySym, struct _rfbClientRec* cl)
{
    InputMousePlatform::keyboardClick(down, KeyCodeSingleton::instance()->getKeyString(keySym));
}

KeyCodeSingleton * KeyCodeSingleton::_instance = nullptr;

KeyCodeSingleton* KeyCodeSingleton::instance() {
    if(_instance == nullptr) {
        _instance = new KeyCodeSingleton();
    }
    return _instance;
}

KeyCodeSingleton::KeyCodeSingleton()
{
    init();
}

KeyCodeSingleton::~KeyCodeSingleton()
{
    delete _instance;
}

void KeyCodeSingleton::init()
{
    const std::string pathCodeKey = ShareDTHome::instance()->getHome() + CODEKEY_PATH;
    std::ifstream infile(pathCodeKey);

    LOGGER.info() << "Loading keyboard code file=" << pathCodeKey;

    std::string line;
    while (std::getline(infile, line))
    {
        parseLine(line, _codeMap);
    }
}

const std::string & KeyCodeSingleton::getKeyString(uint32_t key)
{
    if(_codeMap.find(key) == _codeMap.end()) {
        LOGGER.warn() << "Can't find input key_value=" << key << ". Ignored.";
        return CODEKEY_EMPTY;
    }

    return _codeMap[key];
}

/*
 * file format should be
 *
 * # Key code for MAC
 * # Key = Code(Client)
 * A                    = 65
 * B                    = 66
 * C                    = 67
 * ....
 *
 */
void KeyCodeSingleton::parseLine(const std::string & line, KeycodeString & c)
{
    const char * p = line.c_str();
    while(*p == ' ' || *p == '\t') p++;

    if (*p == '#') return;

    const char * q = p;
    while(*q!=' ' && *q!='\t' &&
          *q!='=' && *q!='\0' && *q!='\n') q++;

    if (p == q) return;
    std::string key(p, q-p);

    while(*q == ' ' || *q == '\t') q++;

    if (*q != '=') {
        LOGGER.warn() << "Invalid KeyCode.txt line:" << line;
        return;
    }
    q++;

    while(*q == ' ' || *q == '\t') q++;

    p = q;
    while(*q <= '9' && *q >= '0') q++;

    if (p == q) {
        LOGGER.warn() << "Invalid KeyCode.txt line:" << line;
        return;
    }

    std::string value(p, q-p);

    uint32_t v = std::stoi(value);

    LOGGER.info() << "Parsed KeyCode.txt key=" << key << " keycode=" << v;
    c[v] = key;
}
