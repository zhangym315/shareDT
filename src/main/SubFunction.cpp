#include "CaptureServer.h"
#include "Capture.h"
#include "TypeDef.h"
#include "SubFunction.h"
#include "MainService.h"
#include "Logger.h"
#include "Path.h"
#include "ReadWriteFD.h"
#include "Sock.h"
#include "service/RemoteGetter.h"
#include "main/client/ShareDTClientWin.h"

#include <QApplication>
#ifdef __SHAREDT_WIN__
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <lmcons.h>
#else
#include "Daemon.h"
#endif

/*
 * Command line to inform service to create child
 * process to run the server procedure
 */
int connectServiceToAction(const char * execCmd, const CaptureServer & cap)
{
    std::string alive = ShareDTHome::instance()->getHome() + std::string(MAIN_SERVER_PATH) + std::string(PATH_ALIVE_FILE);

    int port; std::string host;
    if (cap.getHost().empty()) {
        Path aliveReader(alive, std::fstream::in);
        port = aliveReader.readLineAsInt();
        host = LOCALHOST;
    } else {
        port = SHAREDT_INTERNAL_PORT_START;
        host = cap.getHost();
    }

    SocketClient sc(host, port);
    if (!sc.connect()) {
        fprintf(stderr, "Failed to connect server process.");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    sc.sendString(execCmd);

    std::string receive = sc.receiveStrings();
    fprintf(stdout, ("%s\n"), receive.c_str());

    return RETURN_CODE_SUCCESS;
}

int mainInform(const char * command, const struct cmdConf * conf, const CaptureServer & cap)
{
    /*
     * 1. set home directory
     * 2. check if main service has been started or not
     * 3. set main service file(log pid file)
     */
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted() )
        return RETURN_CODE_INTERNAL_ERROR;

    std::string commandPath;
    commandPath.append(ShareDTHome::instance()->getArgv0());
    commandPath.append(" ");
    commandPath.append(command);

    for (int i=2; i < conf->argc; i++) {
        commandPath.append(" ");
        commandPath.append(conf->argv[i]);
    }

    return connectServiceToAction(commandPath.c_str(), cap);
}

int mainStart (struct cmdConf * conf)
{
    if(conf->argc == 2)
    {
        fprintf(stdout, "Starting ShareDT Server\n");
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

        hSc = OpenService(serviceControlManager, reinterpret_cast<LPCWSTR>(SHAREDT_SERVER_SVCNAME), SERVICE_ALL_ACCESS);
        if (hSc == nullptr)
        {
            LOGGER.error() << "Failed to open service";
            return RETURN_CODE_INTERNAL_ERROR;
        } else if(!StartService(hSc, conf->argc, reinterpret_cast<LPCWSTR *>((const char **) conf->argv)))
        {
            LOGGER.error() << "Failed to start server service";
            if (hSc != nullptr) CloseServiceHandle (hSc);
            return RETURN_CODE_INTERNAL_ERROR;
        }

        if (hSc != nullptr) CloseServiceHandle (hSc);
        fprintf(stdout, "ShareDT Server Started\n");
        return RETURN_CODE_SUCCESS;
#else
        fprintf(stdout, "ShareDT Server Started\n");

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
        return mainInform(SHAREDT_SERVER_COMMAND_START, conf, CaptureServer());
    }

}

int mainStop (struct cmdConf * conf)
{
    if(!setMainProcessServiceHome(conf) ||
       !checkMainServiceStarted())
        return RETURN_CODE_INTERNAL_ERROR;

    if(conf->argc > 2)
    {
        fprintf(stdout, "Stopping Capture Server\n");
        return mainInform(SHAREDT_SERVER_COMMAND_STOP, conf, CaptureServer());
    }

#ifdef __SHAREDT_WIN__
    fprintf(stdout, "Stopping ShareDT Server\n");

    SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CONNECT );
    SC_HANDLE hSc;
    SERVICE_STATUS ServiceStatus;

    hSc = OpenService(serviceControlManager, reinterpret_cast<LPCWSTR>(SHAREDT_SERVER_SVCNAME), SERVICE_STOP  );
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
    fprintf(stdout, "ShareDT Server stopped.\n");
    return RETURN_CODE_SUCCESS;
#else
    fprintf(stdout, "Stopping ShareDT Server\n");
    return connectServiceToAction (MAIN_SERVICE_STOPPING, CaptureServer());
#endif
}

int mainRestart (struct cmdConf * conf)
{
    return RETURN_CODE_SUCCESS;
}

/*
 * New capture request from command
 * This function doesn't start capture server straightforwardly
 * Instead, if notify MainService to start new process as Capture Server
 */
int mainCapture (struct cmdConf * conf)
{
    CaptureServer cap;
    char ** argv = const_cast<char **>(conf->argv);
    if (cap.initParsing(conf->argc, argv) == RETURN_CODE_INVALID_ARG)
        return RETURN_CODE_INVALID_ARG;

#ifdef __SHAREDT_WIN__
    std::string commandPath;
    char szPath[MAX_PATH];
    if( !GetModuleFileNameA(nullptr, reinterpret_cast<LPSTR>(szPath), MAX_PATH ) )
    {
        fprintf(stderr, "Failed to get path of current running command\n");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!setMainProcessServiceHome(conf))
    {
        LOGGER.error () << "Failed to get MainService Home path" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    commandPath.append(reinterpret_cast<const char *const>(szPath));
    commandPath.append(" newCapture");

    for (int i=2; i < conf->argc; i++) {
        commandPath.append(" ");
        commandPath.append(conf->argv[i]);
    }

    // append user name
    char username[UNLEN+1];
    bzero(username, UNLEN+1);
    DWORD username_len = UNLEN;
    GetUserName(reinterpret_cast<LPWSTR>(username), &username_len);
    commandPath.append(" --username \"");
    commandPath.append(username);
    commandPath.append("\"");

    fprintf(stdout, "Starting Capture Server\n");
    connectServiceToAction(commandPath.c_str(), CaptureServer());
    return RETURN_CODE_SUCCESS;
#else
    return mainInform(SHAREDT_SERVER_COMMAND_NEWCAPTURE, conf, cap);
#endif
}

/* main service fork/create new process as the capture server */
int mainNewCapture (struct cmdConf * conf)
{
    CaptureServer cap;
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
        std::string failed = "Failed to start Capture Server.";
        alivePath.write(failed);
        return RETURN_CODE_INTERNAL_ERROR;
    }

    LOGGER.info() << "Write to MainManagementProcess: successfully created capture Server";
    std::string result = StartingCaptureServerMsg::SUCCESS_KEYWORD;
    result.append(std::to_string(cap.getPort()));

    alivePath.write(result);
    LOGGER.info() << "Write result(" << result << ") to file: " << cap.getAlivePath();

    cap.startCaptureServer ();

    return RETURN_CODE_SUCCESS;
}

int mainShow (struct cmdConf * conf)
{
    Capture cap;
    int ret;

    if( (ret=cap.initParsing(conf->argc, const_cast<char **>(conf->argv)) == RETURN_CODE_SUCCESS_SHO) ||
        RETURN_CODE_SUCCESS == ret ) {
        return RETURN_CODE_SUCCESS;
    } else {
        return RETURN_CODE_INTERNAL_ERROR;
    }
}

int noDaemon (struct cmdConf * conf)
{
    int ret;
    CaptureServer cap;
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

int status (struct cmdConf * conf)
{
#ifdef __SHAREDT_WIN__
    std::string commandPath;
    char szPath[MAX_PATH];

    if( !GetModuleFileNameA(nullptr, reinterpret_cast<LPSTR>(szPath), MAX_PATH ) )
    {
        fprintf(stderr, "Failed to get path of current running command\n");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!setMainProcessServiceHome(conf))
    {
        LOGGER.error () << "Failed to get MainService Home path" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    commandPath.append(reinterpret_cast<const char *const>(szPath));
    commandPath.append(" status");

    fprintf(stdout, "Capture Server status:\n");

    connectServiceToAction(commandPath.c_str(), CaptureServer());
    return RETURN_CODE_SUCCESS;
#else
    fprintf(stdout, "Capture Server status:\n");
    return mainInform(SHAREDT_SERVER_COMMAND_STATUS, conf, CaptureServer());
#endif
}

int getSc (struct cmdConf * conf)
{
    SocketClient sc(conf->argv[2], SHAREDT_INTERNAL_PORT_START);

    if (!sc.connect()) {
        return -1;
    }

    std::string cmd = conf->argv[0];

    cmd.append(" ");
    cmd.append(SHAREDT_SERVER_COMMAND_REMOTGET);
    sc.sendString(cmd);

    FrameBuffer fb;
    while (true) {
        RemoteGetterMsg msg{};
        if (sc.receiveBytes((unsigned char *) &msg, sizeof(msg)) <= 0) break;

        msg.convert();

        fb.reSet(msg.dataLen);
        if (sc.receiveBytes(fb.getDataToWrite(), msg.dataLen) < 0 ) break;

        std::cout << "Received frame buffer size=" << msg.dataLen
                  << " name=" << msg.name
                  << " cmdArgs=" << msg.cmdArgs << std::endl;
    }

    std::cout << "Read finished" << std::endl;

    return 0;
}

int connectRemote (struct cmdConf * conf) {
    QApplication app(conf->argc, conf->argv);
    ShareDTClientWin gui(conf->argc, conf->argv);

    if (!gui.isInited()) {
        std::cerr << "Failed to connect to server" << std::endl;
        std::cerr << std::endl;
        return -1;
    }

    gui.show();
    gui.startVNC();

    return QApplication::exec();
}

#ifdef __SHAREDT_WIN__
int installService (struct cmdConf * conf)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    char szPath[MAX_PATH];

    if( !GetModuleFileNameA(nullptr, reinterpret_cast<LPSTR>(szPath), MAX_PATH ) )
    {
        fprintf(stderr, "Cannot install service (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    // Get a handle to the SCM database.
    schSCManager = OpenSCManager(
            nullptr,                    // local computer
            nullptr,                    // ServicesActive database
            SC_MANAGER_ALL_ACCESS);  // full access rights
    if (nullptr == schSCManager)
    {
        fprintf(stderr, "OpenSCManager failed (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    std::string runningServicePath(reinterpret_cast<const char *const>(szPath));
    runningServicePath.insert(0, "\"");
    runningServicePath.insert(runningServicePath.length(), "\"");
    runningServicePath.append(" service");

    schService = CreateService(schSCManager, reinterpret_cast<LPCWSTR>(SHAREDT_SERVER_SVCNAME),
                               reinterpret_cast<LPCWSTR>(SHAREDT_SERVER_SVCNAME),
                               SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                               SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
                               reinterpret_cast<LPCWSTR>(runningServicePath.c_str()),
                               nullptr, nullptr, nullptr, nullptr, nullptr);
    if (schService == nullptr)
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

int uninstallService (struct cmdConf * conf)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus;

    // Get a handle to the SCM database.
    schSCManager = OpenSCManager(
            nullptr,                    // local computer
            nullptr,                    // ServicesActive database
            SC_MANAGER_ALL_ACCESS);  // full access rights
    if (nullptr == schSCManager)
    {
        fprintf(stderr, "OpenSCManager failed (%d)\n", GetLastError());
        return RETURN_CODE_SERVICE_ERROR;
    }

    schService = OpenServiceA( schSCManager, SHAREDT_SERVER_SVCNAME, DELETE);
    if (schService == nullptr)
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

int startService (struct cmdConf * conf)
{
    setMainProcessServiceHome(conf);
    setMainServiceFile() ;
    MainWindowsServices();

    return RETURN_CODE_SUCCESS;
}
#endif

