#include "InputInterface.h"
#include "Logger.h"


void ptrServerMouseEvent(int buttonMask, int x, int y, rfbClientPtr cl)
{
//LOGGER.info("received button: %x, x:%d, y:%d", buttonMask, x, y);
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