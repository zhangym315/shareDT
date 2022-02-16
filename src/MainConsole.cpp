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
#include "Sock.h"
#include "ExportImages.h"

#include <fcntl.h>
#ifdef __SHAREDT_WIN__
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <Lmcons.h>
#else
#include "Daemon.h"
#endif

static int mainInform(const char * command, const struct cmdConf * conf)
{
    /*
     * 1. set home directory
     * 2. check if main service has been started or not
     * 3. set main service file(log pid file)
     */
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted() )
        return RETURN_CODE_INTERNAL_ERROR;

    String commandPath;
    commandPath.append(ShareDTHome::instance()->getHome());
    commandPath.append(MAIN_SERVER_EXEC);
    commandPath.append(command);

    for (int i=2; i < conf->argc; i++) {
        commandPath.append(" ");
        commandPath.append(conf->argv[i]);
    }

    return infoServiceToAction(commandPath.c_str());
}

static int mainStart (const char ** cmdArg, const struct cmdConf * conf)
{
    (void) cmdArg;
    if(conf->argc == 2)
    {
        fprintf(stdout, "Starting shareDTServer\n");
#ifdef __SHAREDT_WIN__
        /*
         * 1. set home directory
         * 2. check if main service has been started or not
         * 3. set main service file(log pid file)
         */
        if(!setMainProcessServiceHome(conf) ||
           !checkMainServiceStarted() )
        {
            LOGGER.error() << "Failed to check process service home";
            return RETURN_CODE_INTERNAL_ERROR;
        }

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
        fprintf(stdout, "ShareDTServer Started\n");
        return RETURN_CODE_SUCCESS;
#else
        fprintf(stdout, "ShareDTServer Started\n");

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
    } else {
        fprintf(stdout, "Starting Capture Server\n");
        return mainInform(" start", conf);
    }

}

static int mainStop (const char ** cmdArg, const struct cmdConf * conf)
{
    (void) cmdArg;
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted())
        return RETURN_CODE_INTERNAL_ERROR;

    if(conf->argc > 2)
    {
        fprintf(stdout, "Stopping Capture Server\n");
        return mainInform(" stop", conf);
    }

#ifdef __SHAREDT_WIN__
    fprintf(stdout, "Stopping ShareDTServer\n");

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
    fprintf(stdout, "ShareDTServer stopped.\n");
    return RETURN_CODE_SUCCESS;
#else
    fprintf(stdout, "Stopping ShareDTServer\n");
    return infoServiceToAction (MAIN_SERVICE_STOPPING);
#endif
}

static int mainRestart (const char ** cmdArg, const struct cmdConf * conf)
{
    return RETURN_CODE_SUCCESS;
}

/*
 * New capture request from command
 * This function doesn't start capture server straightforwardly
 * Instead, if notify MainService to start new process as Capture Server
 */
static int mainCapture (const char ** cmdArg, const struct cmdConf * conf)
{
    StartCapture cap;
    char ** argv = const_cast<char **>(conf->argv);
    if (cap.initParsing(conf->argc, argv) == RETURN_CODE_INVALID_ARG)
        return RETURN_CODE_INVALID_ARG;

#ifdef __SHAREDT_WIN__
    String commandPath;
    TCHAR szPath[MAX_PATH];
    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        fprintf(stderr, "Failed to get path of current running command\n");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!setMainProcessServiceHome(conf))
    {
        LOGGER.error () << "Failed to get MainService Home path" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    commandPath.append(szPath);
    commandPath.append(" newCapture");

    for (int i=2; i < conf->argc; i++) {
        commandPath.append(" ");
        commandPath.append(conf->argv[i]);
    }

    // append user name
    char username[UNLEN+1];
    bzero(username, UNLEN+1);
    DWORD username_len = UNLEN;
    GetUserName(username, &username_len);
    commandPath.append(" --username \"");
    commandPath.append(username);
    commandPath.append("\"");

    fprintf(stdout, "Starting Capture Server\n");
    infoServiceToAction(commandPath.c_str());
#else
    return mainInform(" newCapture", conf);
#endif
}

/* main service fork/create new process as the capture server */
int mainNewCapture (const char ** cmdArg, const struct cmdConf * conf)
{
    (void) cmdArg;
    StartCapture cap;
    char ** argv = const_cast<char **>(conf->argv);
    int ret = cap.initParsing(conf->argc, argv) ||
              cap.initSrceenProvider() ||
              cap.initRFBServer(conf->argc, argv);
    Path alivePath(cap.getAlivePath(), std::fstream::out);

    LOGGER.info() << "Capture Server init finished with ret: " << ret;
    /*
     * If RETURN_CODE_SUCCESS_SHO show window handler
     * return current process
     */
    if(ret == RETURN_CODE_SUCCESS_SHO)
    {
        alivePath.write(EMPTY_STRING);
        return RETURN_CODE_SUCCESS;
    } else if (ret != RETURN_CODE_SUCCESS)
    {
        String failed = "Failed to start Capture Server.";
        alivePath.write(failed);
        return RETURN_CODE_INTERNAL_ERROR;
    }

    LOGGER.info() << "Write to MainManagementProcess: successfully created capture Server";
    String result("Successfully created Capture Server on port: ");
    result.append(std::to_string(cap.getPort()));

    alivePath.write(result);
    LOGGER.info() << "Write result(" << result << ") to file: " << cap.getAlivePath();

    cap.startCaptureServer ();

    return RETURN_CODE_SUCCESS;
}

static int mainShow (const char ** cmdArg, const struct cmdConf * conf)
{
    (void) cmdArg;
    StartCapture cap;
    int ret;

    if( (ret=cap.initParsing(conf->argc, const_cast<char **>(conf->argv)) == RETURN_CODE_SUCCESS_SHO) ||
        RETURN_CODE_SUCCESS == ret ) {
        return RETURN_CODE_SUCCESS;
    } else {
        return RETURN_CODE_INTERNAL_ERROR;
    }
}

static int noDaemon (const char ** cmdArg, const struct cmdConf * conf)
{
    (void) cmdArg;
    int ret;
    StartCapture cap;
    char ** argv = const_cast<char **>(conf->argv);
    ret = cap.initParsing(conf->argc, argv) ||
          cap.initSrceenProvider() ||
          cap.initRFBServer(conf->argc, argv);

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

static int status (const char ** cmdArg, const struct cmdConf * conf)
{
    (void) cmdArg;
#ifdef __SHAREDT_WIN__
    String commandPath;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        fprintf(stderr, "Failed to get path of current running command\n");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!setMainProcessServiceHome(conf))
    {
        LOGGER.error () << "Failed to get MainService Home path" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    commandPath.append(szPath);
    commandPath.append(" status");

    fprintf(stdout, "Capture Server status:\n");

    infoServiceToAction(commandPath.c_str());

#else
    fprintf(stdout, "Capture Server status:\n");
    return mainInform(" status", conf);
#endif
}

#ifdef __SHAREDT_WIN__
static int installService (const char ** cmdArg, const struct cmdConf * conf)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        fprintf(stderr, "Cannot install service (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    // Get a handle to the SCM database.
    schSCManager = OpenSCManager(
            NULL,                    // local computer
            NULL,                    // ServicesActive database
            SC_MANAGER_ALL_ACCESS);  // full access rights
    if (NULL == schSCManager)
    {
        fprintf(stderr, "OpenSCManager failed (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    String runningServicePath(szPath);
    runningServicePath.insert(0, "\"");
    runningServicePath.insert(runningServicePath.length(), "\"");
    runningServicePath.append(" service");

    schService = CreateService(schSCManager, SHAREDT_SERVER_SVCNAME,
                SHAREDT_SERVER_SVCNAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, runningServicePath.c_str(),
                NULL, NULL, NULL, NULL, NULL);
    if (schService == NULL)
    {
        fprintf(stderr, "CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return RETURN_CODE_SERVICE_ERROR;
    }
    else fprintf(stdout, "Service installed successfully\n");

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
        fprintf(stderr, "OpenSCManager failed (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    schService = OpenServiceA( schSCManager, SHAREDT_SERVER_SVCNAME, DELETE);
    if (schService == NULL)
    {
        fprintf(stderr, "OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return RETURN_CODE_SERVICE_ERROR;
    }

    if (! DeleteService(schService) )
    {
        fprintf(stderr, "DeleteService failed (%d)\n", GetLastError());
    }
    else fprintf(stdout, "Service deleted successfully\n");

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
        "                     status\n"
        "                     export\n"
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
        int (*func)(const char **extra, const struct cmdConf *cconf);
    } cmdHandlers[] = {
            { "start" ,     &mainStart   },     /* start service         */
            { "stop"  ,     &mainStop    },     /* stop  service         */
            { "restart",    &mainRestart },     /* restart service       */
            { "capture",    &mainCapture },     /* capture command       */
            { "newCapture", &mainNewCapture },  /* new capture process   */
            { "show",       &mainShow    },     /* command show win      */
            { "nodaemon",   &noDaemon    },     /* run in no daemon mode */
            { "status",     &status      },     /* status of current pro */
            { "export",     &mainExport  }      /* cli to export images  */
#ifdef  __SHAREDT_WIN__
           ,{ "install",    &installService },  /* install service       */
            { "service",    &startService },    /* from scm service      */
            { "uninstall",  &uninstallService } /* uninstall service     */
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
