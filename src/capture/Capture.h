
#ifndef __STARTSERVER_H__
#define __STARTSERVER_H__

#include "WindowsProvider.h"
#include "SamplesProvider.h"
#include "ReadWriteFD.h"
#include "Thread.h"
#include "Path.h"
#include "ScopedPrt.h"

#include "ScreenProvider.h"
#include <iostream>

#define DEFAULT_BOUNDS_LEFT   0
#define DEFAULT_BOUNDS_TOP    0
#define DEFAULT_BOUNDS_RIGHT  1024
#define DEFAULT_BOUNDS_BOTTOM 1024

#define RETURN_CODE_SUCCESS      0
#define RETURN_CODE_SUCCESS_SHO  1
#define RETURN_CODE_INVALID_RFB  2
#define RETURN_CODE_INVALID_ARG  4
#define RETURN_CODE_INTERNAL_ERROR 8
#define RETURN_CODE_SERVICE_ERROR  16
#define RETURN_CODE_CANNOT_PARSE   32

class CommandChecker : public Thread
{
  public:
    CommandChecker(std::string path) : Thread(false), _path(path) { }

    void mainImp();

  private:
    CommandChecker();
    std::string  _path;

};

class Capture {
  public:
    Capture() : _pid(-1), _hdler(0), _show(S_NONE),
         _type(SP_NULL), _sp(nullptr), _monID(0), _daemon(false),
         _ctype(C_NONE), _host{}, _vncPort{},
         _frequency(DEFAULT_SAMPLE_PROVIDER) { }
    ~Capture();

    int initSrceenProvider();
    int initParsing(int argc, char * argv[]);

    enum SType { S_NONE, S_WIN_ALL, S_WIN_NAME, S_MONITOR , S_ALL};
    enum CType { C_CAPTURE, C_NEWCAPTURE, C_START, C_STOP, C_STOP_ALL_SC, C_RESTART, C_SHOW, C_STATUS, C_EXPORT, C_REMOTEGET, C_LOCALDISPLAYER, C_NONE };

    bool setWorkingDirectory();
    void initDaemon();
    void show();             /* show handler for all of windows */

    std::string & setAndGetWID();
    const std::string & getWID() { return _wID; }
    void setWID(const std::string & wid) { _wID = wid ; }

    const std::string & getAlivePath() { return _alivePath; }
    Capture::CType getCType();
    void setCType(CType c) { _ctype = c; }
    void  removeAlivePath() const;

    const std::string & getUserName() const { return _user; }
    const std::string & getCapServerPath() const { return _capturePath; }

    bool isDaemon() const { return _daemon; }
    int  getPort()  const { return _vncPort; }
    std::string  getHost()  const { return _host; }
    unsigned int  getFrenquency()  const { return _frequency; }
    SPType  getType()  const { return _type; }

    ScreenProvider * getScreenProvide() { return _sp; }
    const std::string & getName() const { return _name; }
    void Usage();
    virtual void rfbUsagePrint() { }

  protected:
    ScreenProvider * _sp;     /* screen provider */
    unsigned int     _frequency;

  private:
    int parseArgs(const vector<std::string> & args);
    bool parseBounds();
    bool parseWindows();
    int  parseType();

    /* capture instance */
    union CaptureInstance {
        shared_ptr<Windows> _win;
        CapMonitor          _mon;
        CapImageRect        _bounds; /* this can be used by both partia and
                                      * window and monitor, the later two can
                                      * be used to determine the window and
                                      * monitor bound
                                      */
        CaptureInstance () { }
        ~CaptureInstance () { }
    } _cap;

    SPType           _type;
    std::string      _name;   /* captured named  */
    pid_t            _pid;    /* for window capture, the process id we want to capture */
    size_t           _hdler;  /* for window capture, the handler id we want to capture */
    SType            _show;   /* Screen capture type, monitor, windows or bounds       */
    int              _monID;  /* for monitor capture, the id of monitor */
    bool             _daemon;
    std::string      _wID;    /* unique id */
    std::string      _user;
    std::string      _capturePath;
    std::string      _alivePath;
    CType            _ctype;       /* command type, capture, newcapture, start, stop ... */
    std::string      _host;
    int              _vncPort;

    std::vector<std::string>  _unrecognizedOptions;
};

#endif