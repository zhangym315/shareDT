#ifndef SHAREDT_CAPTURESERVER_H
#define SHAREDT_CAPTURESERVER_H
#include "Capture.h"
#include <rfb/rfb.h>

class CaptureServer : public Capture {
public:
    int initRFBServer(int argc, char *argv[]);
    int getVNCClientCount(struct _rfbClientRec* head);
    void startCaptureServer();

    void rfbUsagePrint() override;
private:
    /* rbf related */
    rfbScreenInfoPtr _rfbserver{};
};

#endif //SHAREDT_CAPTURESERVER_H
