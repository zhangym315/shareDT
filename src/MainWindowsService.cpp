#include "MainService.h"
#include "MainConsole.h"
#include "TypeDef.h"
#include "Logger.h"
#include "Sock.h"

#include <windows.h>
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
    ServiceTable[0].lpServiceName = SHAREDT_SERVER_SVCNAME;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;

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
    if (lpvParam == NULL )
    {
        LOGGER.error() << "InstanceThread got an unexpected NULL" <<
                       " value in lpvParam. InstanceThread exitting.";
        return (DWORD)-1;
    }

    FdBuffer * p = (FdBuffer * ) lpvParam;
    Socket   * s = (Socket  *) (p->fd);

    LOGGER.info() <<"Started new thread for processing CMD=\"" << p->buf << "\"";
    HandleCommandSocket(s, p->buf);

    delete s;
    LOGGER.info() <<"Thread exiting for processing CMD:\"" << p->buf << "\"";

    return 1;
}

void ServiceMain(int argc, char** argv)
{
    LOGGER.info() << "ShareDTServer service is starting" ;

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
    LOGGER.info() << "ShareDTServer service Started";

    /*
     * do the main service work
     */
    String  lpszPipename = SERVICE_PIPE_SERVER;
    HANDLE  hThread;
    DWORD   dwThreadId = 0;
    char    buf[BUFSIZE];

    /* Main service port */
    SocketServer ss(SHAREDT_INTERNAL_PORT_START, 10);
    LOGGER.info() << "MainService started on port=" << ss.getPort() ;
    String alive = ShareDTHome::instance()->getHome() + String(MAIN_SERVER_PATH) + String(PATH_ALIVE_FILE);
    {
        if(fs::exists(alive) && !fs::remove(alive)){
            String error = "Failed to remove the file: " + alive;
            return;
        }
        Path aliveWriter(alive);
        aliveWriter.write(ss.getPort());
    }

    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        Socket* s=ss.Accept();
        String received = s->ReceiveBytes();
        LOGGER.info("ShareDTServer service DATA RECEIVED CMD=\"%s\", clientSocket=%d", received.c_str(), ss.getSocket());

        strcpy_s(buf, received.c_str());
        FdBuffer fb(s, buf);
        LOGGER.info() << "Client connected, creating a processing thread.";
        hThread = CreateThread(NULL, 0, InstanceThread, (LPVOID) &fb, 0, &dwThreadId);
        if (hThread == NULL)
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
            LOGGER.info() << "ShareDTServer service stopped";
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

/*
 * Command line to inform service to create child
 * process to run the server procedure
 */
int infoServiceToAction(const char * execCmd)
{
    String alive = ShareDTHome::instance()->getHome() + String(MAIN_SERVER_PATH) + String(PATH_ALIVE_FILE);
    Path aliveReader(alive);
    int port = aliveReader.readLineAsInt();
    SocketClient sc(LOCALHOST, port);

    fprintf(stdout, "Starting Capture Server\n");

    sc.SendBytes(execCmd);

    String receive = sc.ReceiveBytes();
    fprintf(stdout, TEXT("%s\n"), receive.c_str() );

    return RETURN_CODE_SUCCESS;
}