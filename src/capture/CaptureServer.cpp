#include "Logger.h"
#include "CaptureServer.h"
#include "InputInterface.h"
#include "CaptureInfo.h"
#define SERVERNAME "SHAREDT SERVER"

int CaptureServer::getVNCClientCount(struct _rfbClientRec* head)
{
    int ret = 0 ;
    struct _rfbClientRec * ptr = head;

    while (ptr && ptr->next != head)
    {
        ret ++;
        ptr = ptr->next;
    }

    return ret;
}

int CaptureServer::initRFBServer(int argc, char *argv[])
{
    /* init rfb server */
    _rfbserver = rfbGetScreen(&argc, argv,
                              _sp->getBounds ().getWidth (),
                              _sp->getBounds ().getHeight(), 8, 4, 4);
    if (!_rfbserver) {
        LOGGER.error() << "Failed to create RFB server";
        return RETURN_CODE_INVALID_RFB;
    }
    _rfbserver->desktopName = SERVERNAME;
    return RETURN_CODE_SUCCESS;
}


/*
 * Init RFB server
 * Start capture and send data to client
 **/
void CaptureServer::startCaptureServer()
{
    int preConnected = 0;
    int currentConnected = 0;

    if (_sp == nullptr) {
        LOGGER.error() << "Failed to start server";
        return ;
    }

    if(!_sp->startSample()) {
        LOGGER.error() << "Failed to start SampleProvider" ;
        return ;
    }

    /* start new thread to get command from MMP(MainManagementProcess)  */
    CaptureInfo::instance()->setIsRunning(true);

    CommandChecker cc(CapServerHome::instance()->getHome());
    cc.go();

    /* init rfb server to listen on */
    rfbInitServer(_rfbserver);

    int indicator = 0;
    while(!_sp->isSampleReady() && ++indicator < 100) {
        std::this_thread::sleep_for(50ms);
    }
    if (!_sp->isSampleReady()) return;

    FrameBuffer * fb;

    /* loop through events */
    rfbMarkRectAsModified(_rfbserver, 0, 0,
                          _sp->getWidth(), _sp->getHeight());

    _rfbserver->ptrAddEvent = ptrServerMouseEvent;
    _rfbserver->kbdAddEvent = kbdServerKeyEvent;

    LOGGER.info() << "Started CaptureServer with width=" <<  _sp->getWidth()
                  << " height=" << _sp->getHeight();
    while (rfbIsActive(_rfbserver) && CaptureInfo::instance()->isRunning())
    {
        rfbProcessEvents(_rfbserver, 10000);

        /*
         * Check connected client
         * If no connected client, pause the SamplesProvider
         * Else, resume SamplesProvider
         */
        currentConnected = getVNCClientCount(_rfbserver->clientHead);
        if(preConnected != currentConnected) {
            LOGGER.info() << "Current connected clients: " << currentConnected ;

            /* new state, no connected clients */
            if(currentConnected == 0 && !_sp->isSamplePaused()) {
                _sp->samplePause ();
            }
                /* new state, start sample capturing */
            else if (preConnected==0 && _sp->isSamplePaused())
                _sp->sampleResume();
            preConnected = currentConnected;
        }
        if(currentConnected == 0) continue;

        /* get frame buffer and sync to clients */
        fb = _sp->getFrameBuffer();
        if(!fb) {
            continue;
        }
        _rfbserver->frameBuffer = (char *) fb->getData();

        rfbMarkRectAsModified(_rfbserver, 0, 0,
                              _sp->getWidth(), _sp->getHeight());
    }

    removeAlivePath();
}

void CaptureServer::rfbUsagePrint() {
    rfbUsage();
}
