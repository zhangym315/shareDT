
#ifndef __STARTSERVER_H__
#define __STARTSERVER_H__

#include <ScreenProvider.h>
#include <iostream>
#include "WindowsProvider.h"
#include "SamplesProvider.h"
#include <rfb/rfb.h>
#include "Path.h"

#define DEFAULT_BOUNDS_LEFT   0
#define DEFAULT_BOUNDS_TOP    0
#define DEFAULT_BOUNDS_RIGHT  1024
#define DEFAULT_BOUNDS_BOTTOM 1024

#define RETURN_CODE_SUCCESS      0
#define RETURN_CODE_SUCCESS_SHO  1
#define RETURN_CODE_INVALID_RFB -1
#define RETURN_CODE_INVALID_ARG -2
#define RETURN_CODE_INTERNAL_ERROR -3
#define RETURN_CODE_SERVICE_ERROR  -4

class StartCapture {
  public:
    StartCapture() : _pid(-1), _hdler(0), _show(S_NONE),
         _type(SP_NULL), _sp(NULL), _monID(0), _daemon(false){ }

    int init(int argc, char *argv[]) ;
    int init() { return init(0, NULL); }
    int initParsing(int argc, char * argv[]);
    int getVNCClientCount(struct _rfbClientRec* head);
    void startCaptureServer();

    enum SType { S_NONE, S_WIN_ALL, S_WIN_NAME, S_MONITOR };

    bool setWorkingDirectory();
    void initDaemon();
    void show();             /* show handler for all of windows */

    String & setAndGetWID();
    String & getWID() { return _wID; }

  private:
    void Usage();
    int parseArgs(const vector<String> & args);
    bool parseBounds();
    bool parseWindows();
    bool parseMonitor();
    int  parseType();
    void waitSP() { return; } /* ScreenProvider ready ? */

    /* capture instance */
    union Capture {
        shared_ptr<Windows> _win;
        CapMonitor          _mon;
        CapImageRect        _bounds; /* this can be used by both partia and
                                   * window and monitor, the later two can
                                   * be used to determine the window and
                                   * monitor bound
                                   */
        Capture () { }
        ~Capture () { }
    } _cap;

    SPType           _type;
    ScreenProvider * _sp;   /* screen provider */
    String           _name; /* captured named  */
    Pid              _pid;  /* for window capture, the process id we want to capture */
    size_t           _hdler;  /* for window captre, the handler id we want to capture */
    SType            _show;
    int              _monID;  /* for monitor capture, the id of monitor */
    bool             _daemon;
    String           _wID;  /* unique id */

    /* rbf related */
    rfbScreenInfoPtr _rfbserver;
};

#endif