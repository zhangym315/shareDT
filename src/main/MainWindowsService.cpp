#include "MainService.h"
#include "ShareDT.h"
#include "TypeDef.h"
#include "Logger.h"
#include "Sock.h"

#include <Windows.h>
#include <strsafe.h>
#include <string.h>
#include <stdio.h>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);

int MainWindowsServices()
{
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = (LPSTR)SHAREDT_SERVER_SVCNAME;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = nullptr;
    ServiceTable[1].lpServiceProc = nullptr;

    StartServiceCtrlDispatcher(ServiceTable);
    return 0;
}

struct FdBuffer {
    FdBuffer(Socket * s, char * b) : fd(s), buf(b) { }
    Socket * fd;
    char   * buf;
};

/*
 * Read message from command line and pass it to HandleCommandSocket
 */
DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
    if (lpvParam == nullptr )
    {
        LOGGER.error() << "InstanceThread got an unexpected nullptr" <<
                       " value in lpvParam. InstanceThread exitting.";
        return (DWORD)-1;
    }

    FdBuffer * p = (FdBuffer * ) lpvParam;
    Socket   * s = (Socket  *) (p->fd);
    std::string command(p->buf);

    LOGGER.info() <<"Started new thread for processing CMD=\"" << command << "\"";
    HandleCommandSocket(s, p->buf);

    delete s;
    LOGGER.info() <<"Thread exiting for processing CMD:\"" << command << "\"";

    return 1;
}

void ServiceMain(int argc, char** argv)
{
    LOGGER.info() << "ShareDT Server service is starting" ;

    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    hStatus = RegisterServiceCtrlHandler("ShareDT Server", (LPHANDLER_FUNCTION)ControlHandler);
    if (hStatus == (SERVICE_STATUS_HANDLE)0)
    {
        fprintf(stderr, "Failed to registering Control Handler\n");
        return;
    }

    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus (hStatus, &ServiceStatus);

    /* ok, started the windows service */
    LOGGER.info() << "ShareDT Server service Started";

    /*
     * do the main service work
     */
    std::string  lpszPipename = SERVICE_PIPE_SERVER;
    HANDLE  hThread;
    DWORD   dwThreadId = 0;
    char    buf[BUFSIZE];

    /* Main service port */
    SocketServer ss(SHAREDT_INTERNAL_PORT_START, 10);
    if (!ss.isInit()) {
        LOGGER.error() << "Failed to start MainService";
        return;
    }

    LOGGER.info() << "MainService started on port=" << ss.getPort() ;
    std::string alive = ShareDTHome::instance()->getHome() + std::string(MAIN_SERVER_PATH) + std::string(PATH_ALIVE_FILE);
    {
        if(fs::exists(alive) && !fs::remove(alive)){
            LOGGER.error() << "Failed to remove the file: " << alive;
            return;
        }
        Path aliveWriter(alive);
        aliveWriter.write(ss.getPort());
    }

    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        Socket* s=ss.accept();
        std::string received = s->receiveStrings();
        LOGGER.info("ShareDT Server service DATA RECEIVED CMD=\"%s\", clientSocket=%d", received.c_str(), ss.getSocket());

        strcpy_s(buf, received.c_str());
        FdBuffer fb(s, buf);
        LOGGER.info() << "Client connected, creating a processing thread.";
        hThread = CreateThread(nullptr, 0, InstanceThread, (LPVOID) &fb, 0, &dwThreadId);
        if (hThread == nullptr)
        {
            LOGGER.error () << "CreateThread failed to run, GLE=" <<  GetLastError() << " clientSocket=" << ss.getSocket();
            delete s;
            continue;
        }
        else CloseHandle(hThread);
    }

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus( hStatus, &ServiceStatus );
    return;
}

void ControlHandler(DWORD request)
{
    stopAllSC();

    switch(request)
    {
        case SERVICE_CONTROL_STOP:
            LOGGER.info() << "ShareDT Server service stopped";
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;

        case SERVICE_CONTROL_SHUTDOWN:
            LOGGER.info() << "ShareDTServer service stopped";
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;

        default:
            break;
    }

    // Report current status
    SetServiceStatus (hStatus, &ServiceStatus);

    return;
}

