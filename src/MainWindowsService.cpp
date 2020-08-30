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

#define SERVICE_PIPE_SERVER "\\\\.\\pipe\\SamplePipe\\pipeServer"

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

typedef struct FdBuffer {
    FdBuffer(HANDLE * h, char * b) : handle(h), buf(b) { }
    HANDLE * handle;
    char   * buf;
} FdBuffer;

/*
 * Read message from command line and pass it to HandleCommandSocket
 */
DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
    LOGGER.info() << "InstanceThread stared" ;
    if (lpvParam == NULL )
    {
        LOGGER.error() << "ERROR - Pipe Server Failure: InstanceThread got an unexpected NULL" <<
                       " value in lpvParam. InstanceThread exitting.";
        return (DWORD)-1;
    }

    FdBuffer * p = (FdBuffer * ) lpvParam;
    HANDLE hPipe  = *(p->handle);

    LOGGER.info() << "HandleCommandSocket before stared" ;
    HandleCommandSocket(hPipe, p->buf);

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    LOGGER.info() <<"Pipe Connection Thread exiting.";
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
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread;
    DWORD  dwThreadId = 0;
    int    maxFailed = 0;
    int    receivedBytes;
    char   buf[BUFSIZE];
    bool   rc;

    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
    {
        hPipe = CreateNamedPipe(lpszPipename.c_str(), PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
        if (hPipe == INVALID_HANDLE_VALUE)
        {
            if(maxFailed++ > 10) {
                LOGGER.error() << "Reached to max failed time on CreateNamedPipe, main service stopped";
                Sleep(500);
                break;
            }
            LOGGER.error() << "CreateNamedPipe failed, GLE=" << GetLastError();
            continue ;
        }
        if(maxFailed) maxFailed = 0;
        LOGGER.info() << "Pipe Server: Main thread awaiting client connection on: " << lpszPipename;

        /* new connection from command line */
        if (ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED))
        {
            /* read command line info */
            rc = ReadFile(hPipe, buf, BUFSIZE, reinterpret_cast<LPDWORD>(&receivedBytes), NULL);
            if (!rc || receivedBytes == 0)
            {
                LOGGER.error() <<"Pipe Connection Thread ReadFile failed, GLE=" << GetLastError();
                continue;
            }
            buf[receivedBytes] = '\0';
            LOGGER.info("ShareDTServer service DATA RECEIVED = %s, clientSocket=%d", buf, hPipe);

            /* check if it is stopping command */
            if(!memcmp(buf, MAIN_SERVICE_STOPPING, sizeof(MAIN_SERVICE_STOPPING))){
                LOGGER.info() << "Stopping ShareDTServer Service" ;
                break;
            }

            FdBuffer fb(&hPipe, buf);
            LOGGER.info() << "Client connected, creating a processing thread.";
            hThread = CreateThread(NULL, 0, InstanceThread, (LPVOID) &fb, 0, &dwThreadId);
            if (hThread == NULL)
            {
                LOGGER.error () << "CreateThread failed to run, GLE=" <<  GetLastError() << " clientSocket:" << (int)hPipe;
                continue;
            }
            else CloseHandle(hThread);
        }
        else {
            LOGGER.error() << "Can not connect to client, close the pipe" ;
            CloseHandle(hPipe);
        }
    }

    ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus( hStatus, &ServiceStatus );
    return;
}

void ControlHandler(DWORD request)
{
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
    HANDLE hPipe;
    TCHAR  chBuf[BUFSIZE];
    LPTSTR lpszPipename = TEXT(SERVICE_PIPE_SERVER);

    hPipe = CreateFile(lpszPipename, GENERIC_READ |  GENERIC_WRITE,
                       0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
        printf("Could not open pipe to communicate to server. GLE=%d\n", GetLastError() );
        return RETURN_CODE_SERVICE_ERROR;
    }

    SocketFD fd(hPipe);
    printf("Starting capture server\n");

    if(!fd.send(execCmd))
    {
        printf("Faield to WriteFile to service server. GLE=%d\n", GetLastError() );
        return RETURN_CODE_SERVICE_ERROR;
    }

    if (!fd.recv(chBuf, BUFSIZE))
    {
        printf( TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError() );
        return RETURN_CODE_SERVICE_ERROR;
    }

    printf( TEXT("%s\n"), chBuf );
    CloseHandle(hPipe);
    return RETURN_CODE_SUCCESS;
}
