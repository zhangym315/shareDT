#ifndef _FETCHINGDATA_H_
#define _FETCHINGDATA_H_

#include <QObject>
#include <rfb/rfbclient.h>
#include "SDThread.h"
#include "Buffer.h"

#define VNC_BITS_PER_SAMPLE 8
#define VNC_SAMPLS_PER_PIXEL 3
#define VNC_BYTES_PER_PIXEL 4

struct FramePerInfo {
    FrameBuffer frame;
    int w = 0;
    int h = 0;
};

class FetchingDataFromServer : public SDThread {
  Q_OBJECT

  public:
    FetchingDataFromServer(int argc, char **argv);
    ~FetchingDataFromServer();

    void run();
    bool isInited() { return _isInited; }

    static void writeToFile(const char * file, int x, int y, int w, int h, uint8_t * frame);
    static void HandleRectFromServer(rfbClient* client, int x, int y, int w, int h);
    void HandleRect(rfbClient* client);
    FramePerInfo & getFrame() { return _frame; }
    [[nodiscard]] rfbClient * getRfbClient() const { return _client ;}

  signals:
    void sendRect(rfbClient* client);
    void serverConnectionClosedSend();

  private:
    rfbClient* _client;
    bool       _isInited;
    FramePerInfo _frame;
};

#endif //_FETCHINGDATA_H_
