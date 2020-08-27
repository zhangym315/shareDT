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

VOID GetAnswerToRequest( LPTSTR pchRequest,
                         LPTSTR pchReply,
                         LPDWORD pchBytes )
// This routine is a simple function to print the client request to the console
// and populate the reply buffer with a default data string. This is where you
// would put the actual client request processing code that runs in the context
// of an instance thread. Keep in mind the main thread will continue to wait for
// and receive other client connections while the instance thread is working.
{
    LOGGER.info() << "Client Request String: " << String(pchRequest);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
                        pchRequest,     // Command line
                        NULL,           // Process handle not inheritable
                        NULL,           // Thread handle not inheritable
                        FALSE,          // Set handle inheritance to FALSE
                        0,              // No creation flags
                        NULL,           // Use parent's environment block
                        NULL,           // Use parent's starting directory
                        &si,            // Pointer to STARTUPINFO structure
                        &pi )           // Pointer to PROCESS_INFORMATION structure
            )
    {
        LOGGER.error( "CreateProcess failed (%d).\n", GetLastError() );
        return;
    } else {
        LOGGER.error() << "CreateProcess successfully";
    }

    // Check the outgoing message to make sure it's not too long for the buffer.
    if (FAILED(StringCchCopy( pchReply, BUFSIZE, TEXT("default answer from server") )))
    {
        *pchBytes = 0;
        pchReply[0] = 0;
        LOGGER.error() << "StringCchCopy failed, no outgoing message.";
        return;
    }
    *pchBytes = (lstrlen(pchReply)+1)*sizeof(TCHAR);
}

/*
 * Read message from command line and pass it to HandleCommandSocket
 */
DWORD WINAPI InstanceThread(LPVOID lpvParam)
{
    HANDLE hHeap      = GetProcessHeap();
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
    TCHAR* pchReply   = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe  = NULL;

    if (lpvParam == NULL || pchRequest == NULL ||
            pchReply == NULL )
    {
        LOGGER.error() << "ERROR - Pipe Server Failure: InstanceThread got an unexpected NULL" <<
                        " value in lpvParam/pchRequest/pchReply. InstanceThread exitting.";
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    LOGGER.info() << "Pipe Connection Thread created, receiving and processing messages.";
    hPipe = (HANDLE) lpvParam;

    /* read command line info */
    fSuccess = ReadFile(hPipe, pchRequest, BUFSIZE*sizeof(TCHAR), &cbBytesRead, NULL);

    if (!fSuccess || cbBytesRead == 0)
    {
        LOGGER.error() <<"Pipe Connection Thread ReadFile failed, GLE=" << GetLastError();
        WriteFile( hPipe, "Failed to create process file", cbReplyBytes, &cbWritten, NULL);
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return -1;
    }

//    GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);
    HandleCommandSocket(hPipe, pchRequest);

    /* reply back to command line */
    fSuccess = WriteFile( hPipe, pchReply, cbReplyBytes, &cbWritten, NULL);
    if (!fSuccess || cbReplyBytes != cbWritten)
    {
        LOGGER.error() <<"Pipe Connection Thread WriteFile failed, GLE=" <<  GetLastError();
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    HeapFree(hHeap, 0, pchRequest);
    HeapFree(hHeap, 0, pchReply);

    LOGGER.info() <<"Pipe Connection Thread exiting.";
    return 1;
}


void ServiceMain(int argc, char** argv)
{
    LOGGER.info() << "shareDTServer service is starting" ;

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

    LOGGER.info() << "shareDTServer service Started";

    /*
     * do the main service work
     */
    String  lpszPipename = SERVICE_PIPE_SERVER;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread;
    DWORD  dwThreadId = 0;
    int    maxFailed = 0;

    while (ServiceStatus.dwCurrentState ==
           SERVICE_RUNNING)
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
        LOGGER.info() << "Pipe Server: Main thread awaiting client connection on";

        /* new connection from command line */
        if (ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED))
        {
            LOGGER.info() << "Client connected, creating a processing thread.";
            hThread = CreateThread(NULL, 0, InstanceThread, (LPVOID) hPipe, 0, &dwThreadId);
            if (hThread == NULL)
            {
                LOGGER.error () << "CreateThread failed, GLE=" <<  GetLastError();
                return ;
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
            LOGGER.info() << "shareDTServer service stopped";
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;

        case SERVICE_CONTROL_SHUTDOWN:
            LOGGER.info() << "shareDTServer service stopped";
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
