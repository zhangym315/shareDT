
#ifndef __STARTSERVER_H__
#define __STARTSERVER_H__

#include "WindowsProvider.h"
#include "SamplesProvider.h"
#include "ReadWriteFD.h"
#include "Thread.h"
#include "Path.h"
#include "ScopedPrt.h"

#include <ScreenProvider.h>
#include <iostream>
#include <rfb/rfb.h>

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

#define CAPTURE_STOPPING "STOPPING"

class ReadWriteFDThread : public ReadWriteFD, public Thread
{
  public:
    ReadWriteFDThread(const char * path) : ReadWriteFD(path), Thread(false) { }
    ReadWriteFDThread(const char * path, int oflag) : ReadWriteFD(path, oflag), Thread(false) { }

    void mainImp();

  private:
    ReadWriteFDThread();

};

class CommandChecker : public Thread
{
  public:
    CommandChecker(String path) : Thread(false), _path(path) { }

    void mainImp();

  private:
    CommandChecker();
    String  _path;

};

class StartCapture {
  public:
    StartCapture() : _pid(-1), _hdler(0), _show(S_NONE),
         _type(SP_NULL), _sp(NULL), _monID(0), _daemon(false),
         _ctype(C_NONE) { }
    ~StartCapture();

    int init(int argc, char *argv[]) ;
    int init() { return init(0, NULL); }
    int initParsing(int argc, char * argv[]);
    int getVNCClientCount(struct _rfbClientRec* head);
    void startCaptureServer();

    enum SType { S_NONE, S_WIN_ALL, S_WIN_NAME, S_MONITOR };
    enum CType { C_NEWCAPTURE, C_START, C_STOP, C_STOP_ALL_SC, C_RESTART, C_SHOW, C_STATUS, C_EXPORT, C_NONE };

    bool setWorkingDirectory();
    void initDaemon();
    void show();             /* show handler for all of windows */

    String & setAndGetWID();
    const String & getWID() { return _wID; }

    const String & getAlivePath() { return _alivePath; }
    StartCapture::CType getCType();
    void  removeAlivePath() const;

    [[nodiscard]] const String & getUserName() const { return _user; }
    [[nodiscard]] const String & getCapServerPath() const { return _capturePath; }

    [[nodiscard]] bool isDaemon() const { return _daemon; }
    [[nodiscard]] int  getPort()  const { return _vncPort; }

    [[nodiscard]] const StringVec & getUnrecognizedOptions() const { return _unrecognizedOptions; }

  private:
    void Usage();
    int parseArgs(const vector<String> & args);
    bool parseBounds();
    bool parseWindows();
    int  parseType();

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
    ScreenProvider * _sp;     /* screen provider */
    String           _name;   /* captured named  */
    Pid              _pid;    /* for window capture, the process id we want to capture */
    size_t           _hdler;  /* for window capture, the handler id we want to capture */
    SType            _show;
    int              _monID;  /* for monitor capture, the id of monitor */
    bool             _daemon;
    String           _wID;    /* unique id */
    String           _user;
    String           _capturePath;
    String           _alivePath;

    ScopedPtr<ReadWriteFDThread> _listenMMP;

    CType            _ctype;  /* command type, newcaptre, start, stop ... */
    int              _vncPort{};

    StringVec        _unrecognizedOptions;

    /* rbf related */
    rfbScreenInfoPtr _rfbserver{};
};

#endif