/*********************************************************
 * MainConsole.cpp                                       *
 *                                                       *
 * main function to start a server                       *
 *                                                       *
 *********************************************************/

#include "StartServer.h"
#include "TypeDef.h"
#include "MainConsole.h"
#include "MainService.h"
#include "Logger.h"
#include "Path.h"
#include "ReadWriteFD.h"

#include <fcntl.h>
#ifdef __SHAREDT_WIN__
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

void WindowsMainServiceCallback(int argc, char ** argv)
{
    StartCapture cap;
    cap.init(argc, argv);
    cap.startCaptureServer ();
    return;
}
#else
#include "Daemon.h"
#endif

static int mainStart (const char ** cmdArg, const struct cmdConf * conf)
{
    printf("Starting shareDTServer\n");
#ifdef __SHAREDT_WIN__
    /*
     * 1. set home directory
     * 2. check if main service has been started or not
     * 3. set main service file(log pid file)
     */
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted() ||
       !setMainServiceFile() )
        return RETURN_CODE_INTERNAL_ERROR;

    SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CONNECT );
    SC_HANDLE hSc;

    hSc = OpenService(serviceControlManager, SHAREDT_SERVER_SVCNAME, SERVICE_ALL_ACCESS);
    if (hSc == nullptr)
    {
        LOGGER.error() << "Failed to open service";
        return RETURN_CODE_INTERNAL_ERROR;
    } else if(!StartService(hSc, conf->argc, conf->argv))
    {
        LOGGER.error() << "Failed to start server service";
        if (hSc != nullptr) CloseServiceHandle (hSc);
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if (hSc != nullptr) CloseServiceHandle (hSc);
    printf("shareDTServer Started\n");
    return RETURN_CODE_SUCCESS;
#else
    DaemonizeProcess::instance()->daemonize();

    /*
     * 1. set home directory
     * 2. check if main service has been started or not
     * 3. set main service file(log pid file)
     */
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted() ||
       !setMainServiceFile() )
        return RETURN_CODE_INTERNAL_ERROR;

    DaemonizeProcess::instance()->daemonizeInit();
    LOGGER.info() << "Daemonized init finished, pid: " << getpid();

    return MainWindowsServices();
#endif
}

static int mainStop (const char ** cmdArg, const struct cmdConf * conf)
{
    printf("Stopping shareDTServer\n");
#ifdef __SHAREDT_WIN__
    SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CONNECT );
    SC_HANDLE hSc;
    SERVICE_STATUS ServiceStatus;

    hSc = OpenService(serviceControlManager, SHAREDT_SERVER_SVCNAME, SERVICE_STOP  );
    if (hSc == nullptr)
    {
        LOGGER.error() << "Can't stop service";
        return RETURN_CODE_SERVICE_ERROR;
    } else {
        if (!ControlService (hSc, SERVICE_CONTROL_STOP, &ServiceStatus))
        {
            LOGGER.error() <<  "Cannot control service";
            return RETURN_CODE_SERVICE_ERROR;
        }
    }
    if (hSc != nullptr) CloseServiceHandle (hSc);
    return RETURN_CODE_SUCCESS;
#else
    /*
     * 1. set home directory
     * 2. check if main service has been started or not
     * 3. set main service file(log pid file)
     */
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted() )
        return RETURN_CODE_INTERNAL_ERROR;

    return infoServiceToCapture(MAIN_SERVICE_STOPPING);
#endif
}

static int mainRestart (const char ** cmdArg, const struct cmdConf * conf)
{
    return RETURN_CODE_SUCCESS;
}

/* new capture from command */
static int mainCapture (const char ** cmdArg, const struct cmdConf * conf)
{
#ifdef __SHAREDT_WIN__
    String commandPath;
    TCHAR szPath[MAX_PATH];
    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        return RETURN_CODE_INTERNAL_ERROR;
    }

    commandPath.append(szPath);
    commandPath.append(" newCapture");

    for (int i=2; i < conf->argc; i++) {
        commandPath.append(" ");
        commandPath.append(conf->argv[i]);
    }
    infoServiceToCapture(commandPath.c_str());
#else
    String commandPath;
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted() )
        return RETURN_CODE_INTERNAL_ERROR;

    commandPath.append(ShareDTHome::instance()->getHome());
    commandPath.append(MAIN_SERVER_EXEC);
    commandPath.append(" newCapture");

    for (int i=2; i < conf->argc; i++) {
        commandPath.append(" ");
        commandPath.append(conf->argv[i]);
    }

    return infoServiceToCapture(commandPath.c_str());

#endif
}

/* main service fork/create new process as the capture server */
int mainNewCapture (const char ** cmdArg, const struct cmdConf * conf)
{
    StartCapture cap;
    int ret = cap.init(conf->argc, const_cast<char **>(conf->argv));
    String alivePath = CapServerHome::instance()->getHome() + PATH_ALIVE_FILE;

    ReadWriteFD msg(alivePath.c_str(), O_WRONLY);
    /*
     * If RETURN_CODE_SUCCESS_SHO show window handler
     * return current process
     */
    if(ret == RETURN_CODE_SUCCESS_SHO)
    {
        return RETURN_CODE_SUCCESS;
    } else if (ret != RETURN_CODE_SUCCESS)
    {
        msg.write("Failed to create Capture Server");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    msg.write("Successfully created Capture Server");
    cap.startCaptureServer ();

    return RETURN_CODE_SUCCESS;
}

static int mainShow (const char ** cmdArg, const struct cmdConf * conf)
{
    StartCapture cap;
    cap.init(conf->argc, const_cast<char **>(conf->argv));
    cap.show();
    return RETURN_CODE_SUCCESS;
}

static int noDaemon (const char ** cmdArg, const struct cmdConf * conf)
{
    int ret;
    StartCapture cap;
    ret = cap.init(conf->argc, const_cast<char **>(conf->argv));

    /*
     * If RETURN_CODE_SUCCESS_SHO show window handler
     * return current process
     */
    if(ret == RETURN_CODE_SUCCESS_SHO)
        return RETURN_CODE_SUCCESS;
    else if (ret != RETURN_CODE_SUCCESS)
        return ret;

    /* start capture */
    cap.startCaptureServer ();

    return RETURN_CODE_SUCCESS;
}

#ifdef __SHAREDT_WIN__
static int installService (const char ** cmdArg, const struct cmdConf * conf)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        printf("Cannot install service (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    // Get a handle to the SCM database.
    schSCManager = OpenSCManager(
            NULL,                    // local computer
            NULL,                    // ServicesActive database
            SC_MANAGER_ALL_ACCESS);  // full access rights
    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    String runningServicePath(szPath);
    runningServicePath.insert(0, "\"");
    runningServicePath.insert(runningServicePath.length(), "\"");
    runningServicePath.append(" service");
    // Create the service
    schService = CreateService(
            schSCManager,              // SCM database
            SHAREDT_SERVER_SVCNAME,    // name of service
            SHAREDT_SERVER_SVCNAME,    // service name to display
            SERVICE_ALL_ACCESS,        // desired access
            SERVICE_WIN32_OWN_PROCESS, // service type
            SERVICE_DEMAND_START,      // start type
            SERVICE_ERROR_NORMAL,      // error control type
            runningServicePath.c_str(),// path to service's binary
            NULL,                      // no load ordering group
            NULL,                      // no tag identifier
            NULL,                      // no dependencies
            NULL,                      // LocalSystem account
            NULL);                     // no password
    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return RETURN_CODE_SERVICE_ERROR;
    }
    else printf("Service installed successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return RETURN_CODE_SUCCESS;
}

static int uninstallService (const char ** cmdArg, const struct cmdConf * conf)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus;

    // Get a handle to the SCM database.
    schSCManager = OpenSCManager(
            NULL,                    // local computer
            NULL,                    // ServicesActive database
            SC_MANAGER_ALL_ACCESS);  // full access rights
    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    // Get a handle to the service.
    schService = OpenServiceA(
            schSCManager,           // SCM database
            SHAREDT_SERVER_SVCNAME, // name of service
            DELETE);                // need delete access

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return RETURN_CODE_SERVICE_ERROR;
    }

    // Delete the service.
    if (! DeleteService(schService) )
    {
        printf("DeleteService failed (%d)\n", GetLastError());
    }
    else printf("Service deleted successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return RETURN_CODE_SUCCESS;
}

static int startService (const char ** cmdArg, const struct cmdConf * conf)
{
    setMainProcessServiceHome(conf);
    setMainServiceFile() ;
    MainWindowsServices();

    return RETURN_CODE_SUCCESS;
}
#endif

static void Usage()
{
    fprintf(stdout, "%s\n",
        "Usage: ShareDTServer start\n"
        "                     stop\n"
        "                     restart\n"
        "                     capture\n"
        "                     show\n"
        "                     nodaemon\n"
        );
}

int main(int argc, char** argv)
{
    if(argc < 2) {
        Usage();
        return -1;
    }

    static const struct {
        const char *name;
        int (*func)(const char **extra,
                     const struct cmdConf *cconf);
    } cmdHandlers[] = {
            { "start" ,     &mainStart   },     /* start service      */
            { "stop"  ,     &mainStop    },     /* stop  service      */
            { "restart",    &mainRestart },     /* restart service    */
            { "capture",    &mainCapture },     /* capture command    */
            { "newCapture", &mainNewCapture },  /* new capture process */
            { "show",       &mainShow    },     /* command show win   */
            { "nodaemon",   &noDaemon    }      /* command show win   */
#ifdef  __SHAREDT_WIN__
           ,{ "install",    &installService },  /* install service    */
            { "service",    &startService },    /* from scm service   */
            { "uninstall",  &uninstallService } /* uninstall service  */
#endif
    };
    unsigned cmd_count = 0;
    struct cmdConf cconf;
    OS_ALLOCATE(const char *, cmd, argc + 1);
    for (int x = 0; x < argc; x++) {
        cmd[cmd_count++] = argv[x];
    }
    cmd[cmd_count] = NULL;
    cconf.argc = cmd_count;
    cconf.argv = cmd;

    for (int i = 0; i < ARRAY_SIZE(cmdHandlers); i++) {
        if (chars_equal(cmdHandlers[i].name, cmd[1])) {
            int ret = cmdHandlers[i].func(cmd + 1, &cconf);
            fflush(stdout);
            return ret;
        }
    }

    Usage();
    return -1;
}
