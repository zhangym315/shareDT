#include "MainService.h"
#include "MainConsole.h"
#include "TypeDef.h"
#include "Logger.h"

#include <windows.h>
#include <strsafe.h>
#include <string.h>
#include <stdio.h>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

#define SERVICE_PIPE_SERVER "\\\\.\\pipe\\SamplePipe\\pipeServer"
#define BUFSIZE 5120

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

/*
 * command line to inform service to create child
 * process to run the server procedure
 */
int infoServiceToAction(const char * execCmd)
{
    HANDLE hPipe;
    const char *lpvMessage=TEXT(execCmd);
    TCHAR  chBuf[BUFSIZE];
    BOOL   fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    LPTSTR lpszPipename = TEXT(SERVICE_PIPE_SERVER);

    /*  Try to open a named pipe; wait for it, if necessary. */
    while (1)
    {
        hPipe = CreateFile(
                lpszPipename,   // pipe name
                GENERIC_READ |  // read and write access
                GENERIC_WRITE,
                0,              // no sharing
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe
                0,              // default attributes
                NULL);          // no template file

        /* Break if the pipe handle is valid. */
        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        /* Exit if an error other than ERROR_PIPE_BUSY occurs. */
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            printf("Could not open pipe to communicate to server. GLE=%d\n", GetLastError() );
            return RETURN_CODE_SERVICE_ERROR;
        }

        // All pipe instances are busy, so wait for 20 seconds.
        if ( ! WaitNamedPipe(lpszPipename, 20000))
        {
            printf("Could not open pipe: 20 second wait timed out.");
            return RETURN_CODE_SERVICE_ERROR;
        }
    }

    // The pipe connected; change to message-read mode.
    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(
            hPipe,    // pipe handle
            &dwMode,  // new pipe mode
            NULL,     // don't set maximum bytes
            NULL);    // don't set maximum time
    if ( ! fSuccess)
    {
        printf("SetNamedPipeHandleState failed. GLE=%d\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    // Send a message to the pipe server.
    cbToWrite = (lstrlen(lpvMessage)+1)*sizeof(TCHAR);
    printf("Starting capture server\n");
    fSuccess = WriteFile(
            hPipe,                  // pipe handle
            lpvMessage,             // message
            cbToWrite,              // message length
            &cbWritten,             // bytes written
            NULL);                  // not overlapped

    if ( ! fSuccess)
    {
        printf("Faield to WriteFile to service server. GLE=%d\n", GetLastError() );
        return RETURN_CODE_SERVICE_ERROR;
    }

    do
    {
        // Read from the pipe.
        fSuccess = ReadFile(
                hPipe,    // pipe handle
                chBuf,    // buffer to receive reply
                BUFSIZE*sizeof(TCHAR),  // size of buffer
                &cbRead,  // number of bytes read
                NULL);    // not overlapped
        if ( ! fSuccess && GetLastError() != ERROR_MORE_DATA )
            break;

        printf( TEXT("\"%s\"\n"), chBuf );
    } while ( ! fSuccess);  // repeat loop if ERROR_MORE_DATA

    if ( ! fSuccess)
    {
        printf( TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError() );
        return RETURN_CODE_SERVICE_ERROR;
    }
    CloseHandle(hPipe);
    return RETURN_CODE_SUCCESS;
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

    // Do some extra error checking since the app will keep running even if this
    // thread fails.
    if (lpvParam == NULL || pchRequest == NULL ||
            pchReply == NULL )
    {
        LOGGER.error() << "ERROR - Pipe Server Failure:";
        LOGGER.error() << "   InstanceThread got an unexpected NULL value in lpvParam/pchRequest/pchReply.";
        LOGGER.error() << "   InstanceThread exitting.";
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
    HandleCommandSocket((int)hPipe, pchRequest);

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
    int error;
    LOGGER.info() << "shareDTServer service is starting" ;

    ServiceStatus.dwServiceType =
            SERVICE_WIN32;
    ServiceStatus.dwCurrentState =
            SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   =
            SERVICE_ACCEPT_STOP |
            SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    hStatus = RegisterServiceCtrlHandler("ShareDT Server",
                    (LPHANDLER_FUNCTION)ControlHandler);
    if (hStatus == (SERVICE_STATUS_HANDLE)0)
    {
        // Registering Control Handler failed
        return;
    }

    // We report the running status to SCM.
    ServiceStatus.dwCurrentState =
            SERVICE_RUNNING;
    SetServiceStatus (hStatus, &ServiceStatus);

    LOGGER.info() << "shareDTServer service Started";

    /*
     * do the main service work
     */
    String  lpszPipename = SERVICE_PIPE_SERVER;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread;
    DWORD  dwThreadId = 0;
    BOOL   fConnected = FALSE;

    while (ServiceStatus.dwCurrentState ==
           SERVICE_RUNNING)
    {
        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
        hPipe = CreateNamedPipe(
                lpszPipename.c_str(),     // pipe name
                PIPE_ACCESS_DUPLEX,       // read/write access
                PIPE_TYPE_MESSAGE |       // message type pipe
                PIPE_READMODE_MESSAGE |   // message-read mode
                PIPE_WAIT,                // blocking mode
                PIPE_UNLIMITED_INSTANCES, // max. instances
                BUFSIZE,                  // output buffer size
                BUFSIZE,                  // input buffer size
                0,                        // client time-out
                NULL);                    // default security attribute
        if (hPipe == INVALID_HANDLE_VALUE)
        {
            LOGGER.error() << "CreateNamedPipe failed, GLE=" << GetLastError();
            return ;
        }

        LOGGER.info() << "Pipe Server: Main thread awaiting client connection on";
        fConnected = ConnectNamedPipe(hPipe, NULL) ?
                     TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (fConnected)
        {
            LOGGER.info() << "Client connected, creating a processing thread.";
            // Create a thread for this client.
            hThread = CreateThread(
                    NULL,        // no security attribute
                    0,           // default stack size
                    InstanceThread,        // thread proc
                    (LPVOID) hPipe,        // thread parameter
                    0,           // not suspended
                    &dwThreadId);          // returns thread ID
            if (hThread == NULL)
            {
                LOGGER.error () << "CreateThread failed, GLE=" <<  GetLastError();
                return ;
            }
            else CloseHandle(hThread);
        }
        else {
            LOGGER.error() << "Can not connect to client, close the pipe" ;
            // The client could not connect, so close the pipe.
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

