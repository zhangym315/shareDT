#ifndef SHAREDT_MAINSERVICE_H
#define SHAREDT_MAINSERVICE_H
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "StartServer.h"
#include "Sock.h"

#define SERVICE_PIPE_SERVER "\\\\.\\pipe\\SamplePipe\\pipeServer"

#define BUFSIZE 5120
#define MAX_ARG 128

#define MAIN_SERVICE_STOPPING     "MAIN_STOPPING"
#define MAIN_SERVICE_STOPPING_SUC "MAIN_STOPPING_SUCCESSFUL"
#define MAIN_SERVICE_CONTINUING   "MAIN_CONTINUING"

int MainWindowsServices();
int infoServiceToAction(const char * execCmd);
void stopAllSC();

#ifdef __SHAREDT_WIN__
void HandleCommandSocket(Socket * sk, char * buf);
#else
void HandleCommandSocket(int fd, char * buf);
#endif

#ifdef __SHAREDT_WIN__
#else
#include "Sock.h"
#include "Thread.h"

/*
 * Singleton instance
 *
 * This instance hold all of the fd communicating to the
 *      Capture server.
 *
 * And also the Capture Server ID
 *
 * CaptureProcessManager::instance()
 *                     |
 *                     -----  Capture Server 1
 *                     |
 *                     -----  Capture Server 2
 */
class CaptureProcessManager : public Thread
{
  public:
    static CaptureProcessManager * instance() ;
    ~CaptureProcessManager();

    // thread main process
    void mainImp();

  private:
    CaptureProcessManager();
    static CaptureProcessManager * _instance;


};

class MainServiceServer
{
  public:
    MainServiceServer();
    ~MainServiceServer();

    void setValid(bool valid) { _valid = valid; }
    bool getValid()  {return _valid; }
    void removeSocketFile();

    int listening();
    int getNewConnection();

  private:
    int _serverSock;
    int _backlog;
    std::vector<int> _clientsSock;
    bool _valid;
    String _socketFile;
};

class MainServiceClient
{
  public:
    MainServiceClient();
    ~MainServiceClient();

    int sentTo (const char * cmd);
    int rcvFrom(char * buf, size_t size);

    void setValid(bool valid) { _valid = valid; }
    bool getValid()  {return _valid; }

  private:
    int  _clientSock;
    bool _valid;
};
#endif

class HandleCommandLine
{
public:
    HandleCommandLine(char * buf);
    ~HandleCommandLine();

    String toString(int offset);
    String toString() { return toString(0); }
    int initParsing() { return _sc.initParsing(_argc, _argv); }

    void setWID();

    char ** getArgv() { return _argv; }
    int     getArgc() { return _argc; }

    bool hasWid() { return _hasWid; }
    StartCapture & getSC() { return _sc; }

    bool isDaemon() { return _sc.isDaemon(); }
    void setDaemon() ;
    void setVNCPort(int port) ;
private:
    HandleCommandLine();
    char ** _argv;
    int     _argc;
    bool    _hasWid;
    StartCapture _sc;
};

//extern static void setMainProcessServiceHome(const char * execpath);
extern bool setMainProcessServiceHome(const struct cmdConf * conf);
extern bool checkMainServiceStarted();
extern bool setMainServiceFile();

class MainService
{
  public:
    MainService() { };
    ~MainService() { };
private:
};

#endif //SHAREDT_MAINSERVICE_H
