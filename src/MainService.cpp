#include "MainService.h"
#include "Path.h"
#include "Logger.h"
#include "MainConsole.h"
#include "CrossPlatform.h"
#include "MainManagementProcess.h"
#include "Foreach.h"
#include "Sock.h"

#include <iostream>
#include <stdlib.h>

#ifdef __SHAREDT_WIN__
#include "WindowsProcess.h"
#include <windows.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#endif

/*
 * Global WID management
 */
WIDMAP _WIDManager;
std::mutex _WIDManagerMutex;
static unsigned int _incrse   = 0;   // vnc server start port

/*
 * Stopping all CaptureServer in _WIDManager
 */
void stopAllSC()
{
    LOGGER.info() << "Stoping all Capture Server";
    FOREACH(WIDMAP, it, _WIDManager)
    {
        MainManagementProcess::STATUS statusType = it->second.status();

        if(statusType == MainManagementProcess::STATUS::STARTED)
        {
            LOGGER.info() << "Sending stopping to WID: " << it->first;
            it->second.stop();

            std::lock_guard<std::mutex> guard(_WIDManagerMutex);
            it->second.updateStatus(MainManagementProcess::STATUS::STOPPED);
            LOGGER.info() << "Stopped Capture Server WID: " << it->first;
        }
    }
    return;
}

static void stopSpecificCaptureServer(
        Socket * sk,
        HandleCommandLine & hcl )
{
    String wid;

    /* parsing input argument */
    hcl.initParsing();
    wid = hcl.getSC().getWID();
    WIDMAP::iterator it = _WIDManager.find(wid);

    if(!hcl.hasWid()) {
        sk->send("Command must has a valid \"--wid\" setting");
        return ;
    }

    if(it == _WIDManager.end()) {
        String msg("Can't find status for: ");
        msg.append(wid);
        sk->send(msg.c_str());
        return;
    }

    LOGGER.info() << "Sending stopping to WID: " << wid;

    const String & capServerHome = hcl.getSC().getCapServerPath();
    String stop    = capServerHome + PATH_SEP_STR + CAPTURE_SERVER_STOP;
    String stopped = capServerHome + PATH_SEP_STR + CAPTURE_SERVER_STOPPED;

    if(fs::exists(stop) && !fs::remove(stop)){
        LOGGER.error() << "Failed to remove the stop file: " << stop;
    }
    if(fs::exists(stopped) && !fs::remove(stopped)){
        LOGGER.error() << "Failed to remove the stopped file: " << stopped;
    }

    std::ofstream ofs(stop.c_str());
    ofs << "stop";
    ofs.close();

    int i;
    /* wait for 20s for CaptureServer to stop */
    for (i = 0 ; i < 20; i++)
    {
        if(fs::exists(stopped))
            break;
        this_thread::sleep_for(1s);
    }

    /* mark the server as stopped even we don't understand the status */
    std::lock_guard<std::mutex> guard(_WIDManagerMutex);
    it->second.updateStatus(MainManagementProcess::STATUS::STOPPED);

    /* failed to know the CaptureServer */
    if(i == 20)
    {
        LOGGER.error() << "Waited 20s for CaptureServer to stop, but no responds for wid=" << wid;
        String msg("Capture Server Unknown Status: ");
        msg.append(wid);
        sk->send(msg.c_str());
        return;
    }

    /* send msg back to command line */
    String msg("Capture Server Stopped: ");
    msg.append(wid);

    sk->send(msg.c_str());
}

static void statusAllSC (
        Socket * sk,
        HandleCommandLine & hcl )
{
    String ret;
    if(_WIDManager.empty()) {
        sk->send("No Capture Server found.\n");
        return;
    }

    FOREACH_CONST( WIDMAP, it, _WIDManager)
    {
        ret.append("Capture Server on port: ");
        ret.append(std::to_string(it->second.getPort()));
        ret.append("\t Status: ");
        if(it->second.status() == MainManagementProcess::STATUS::STOPPED)
            ret.append("Stopped");
        else if(it->second.status() == MainManagementProcess::STATUS::PENDING)
            ret.append("Pending");
        else
            ret.append("Started");
        ret.append("\t WID: " + it->first + "\n");
    }

    sk->send(ret.c_str());
}

/*
 * This is center of msg handling, it receives message from CLI command, and do the
 * corresponding action, then send back status back to CLI command.
 *
 * New request from MainServiceServer, usually it's
 * command request. Like:
 * ShareDTServer start
 * ShareDTServer capture -c win -h 1234 --daemon --wid WINDOW_1597564504_HID269_RND0278580287
 * ShareDTserver stop --wid WINDOW_1597564504_HID269_RND0278580287
 *
 * 1. New capture, construct the capture id(cid)
 *    a). Generate cid and fork a new process.
 *    b). Add the cid and fd to CaptureProcessManager
 * 2. Stopping existing capture
 *    a). Find out the capture id and fd
 *    b). Send stopping message to capture process
 * 3. Start existing capture id
 *    a). Find out the capture directory
 *    b). Start the capture id
 *    c). Push the fd anc cid to CaptureProcessManager
 * 4. Delete capture id
 *    a). Find out the capture id and fd in CaptureProcessManager
 *    b). Inform capture server process to stop
 *    c). Delete the directory and records in CaptureProcessManager
 *
 * If new capture server reqeust is received
 *
 */
void HandleCommandSocket(Socket * sk, char * buf)
{
    String wid;
    StartCapture::CType commandType;
    /* start handle particular --wid specified or start new capture server */
    HandleCommandLine hcl(buf);

    /* parsing input argument */
    hcl.initParsing();
    wid = hcl.getSC().getWID();
    WIDMAP::iterator it = _WIDManager.find(wid);
    commandType = hcl.getSC().getCType();
    String user = hcl.getSC().getUserName();
    const String & capServerHome = hcl.getSC().getCapServerPath();

    /* 0. check if status command */
    if( commandType == StartCapture::C_STATUS )
    {
        return statusAllSC(sk, hcl);
    }

    /* 1. first check stop specific wid */
    if( commandType == StartCapture::C_STOP )
    {
        return stopSpecificCaptureServer(sk, hcl);
    }
    if( commandType == StartCapture::C_STOP_ALL_SC)
    {
        stopAllSC();
        sk->send("All Capture Server are stopped.");
        return;
    }

    if(!hcl.hasWid())   hcl.setWID();
    if(!hcl.isDaemon()) hcl.setDaemon();

    String ret("Starting Capture ID(CID) = ");
    ret.append(wid);
    ret.append("\nStatus: ");

    /*
     * 2. Starting capture server
     * 2.1 First make sure there is no "start" and "started" file under it.
     */
    if( (commandType == StartCapture::C_START || commandType == StartCapture::C_NEWCAPTURE) &&
        (it == _WIDManager.end() || it->second.status() != MainManagementProcess::STATUS::STARTED) ) {
        String start = capServerHome + String(CAPTURE_SERVER_START);
        String started = capServerHome + String(CAPTURE_SERVER_STARTED);
        String alive = capServerHome + PATH_ALIVE_FILE;
        int    capServerPort = VNCSERVER_PORT_START + _incrse;

        if(fs::exists(start) && !fs::remove(start)){
            LOGGER.error() << "Failed to remove the file: " << start;
        }

        if(fs::exists(started) && !fs::remove(started)){
            LOGGER.error() << "Failed to remove the file: " << started;
        }

        hcl.setVNCPort(capServerPort);
        LOGGER.info() << "Starting Capture Server on port: " << capServerPort << " with Argument: " << hcl.toString();
        int childPid;

        if(fs::exists(alive) && !fs::remove(alive)){
            String error = "Failed to remove the file: " + alive;
            LOGGER.error() << error;
            ret.append(error);
            sk->send(ret.c_str());
            return;
        }
#ifndef __SHAREDT_WIN__
        char ** argv = hcl.getArgv();
        if((childPid=fork()) == 0) {
            execv(argv[0], argv);
        }

#else
        /* create process as the user requested */
        LOGGER.info() << "Retrieving user session token for user=" << user;

        UserSession usrSession(user);
        if(!usrSession.isValid())
        {
            ret.append(usrSession.getReason());
            sk->send(ret.c_str());
            return;
        }

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );
        if(!CreateProcessAsUserA ( usrSession.getToken(),
                                NULL,    // module name (use command line)
                               (LPTSTR)hcl.toString().c_str(),     // Command line
                                NULL,    // Process handle not inheritable
                                NULL,    // Thread handle not inheritable
                                true,    // Set handle inheritance to FALSE
                                CREATE_NO_WINDOW, // no console window
                                NULL,    // Use parent's environment block
                                NULL,    // Use parent's starting directory
                                &si,     // Pointer to STARTUPINFO structure
                                &pi )    // Pointer to PROCESS_INFORMATION structure
                            )
        {
            sk->send("Failed to create child capture process");
            LOGGER.info() << "Failed to create child capture process";
            return;
        }
        CloseHandle(pi.hThread);
        childPid = (int) pi.hProcess;
#endif

        LOGGER.info() << "Successuflly create child process for WID=" << wid <<
                      " started, PID=" << childPid << " CMD=" << hcl.toString();

        String answer;

        if(Path::checkAndWait(alive, 10)) {
            sk->send("Have waited for 10 seconds, but no responds from CaptureServer process.");
            return;
        }

        Path alivePath(alive, std::fstream::in);
        answer = alivePath.readAll();

        ret.append(answer);
        sk->send(ret.c_str());
        LOGGER.info() << "Sent to sorcket: " << sk->getSocket() << " message: " << ret ;

        /*
         * increase the vnc server port even failed
         */
        _incrse++;

        int success = false;
        /* increase the dest port if successfully create Capture Server */
        if(answer.find("Successfully created") != String::npos)
        {
            success = true;
        }

        /* add it to global _WIDManager, skip the failed one */
        if(it == _WIDManager.end() && success) {
            std::lock_guard<std::mutex> guard(_WIDManagerMutex);
            _WIDManager.insert(std::pair<String, MainManagementProcess>
                       (wid, MainManagementProcess(alive, capServerHome, capServerPort,
                                                   MainManagementProcess::STATUS::STARTED)));
        } else if (success) {
            std::lock_guard<std::mutex> guard(_WIDManagerMutex);
            it->second.updateStatus(MainManagementProcess::STATUS::STARTED);
        }
    } else {
        ret += "Already started";
        sk->send(ret.c_str());
    }

}

HandleCommandLine::HandleCommandLine(char * buf) : _hasWid(false)
{
    char ** argv = (char **) malloc (sizeof(char * ) * MAX_ARG);
    char * token = strtok(buf, " ");
    int i = 0;

    // loop through the string to extract all other tokens
    while( token != NULL ) {
        argv[i++] = strdup(token);
        if(!strcmp(argv[i-1], "--wid"))
            _hasWid = true;
        token = strtok(NULL, " ");
    }

    argv[i] = NULL;
    _argv = argv;
    _argc = i;
}

HandleCommandLine::~HandleCommandLine()
{
    for(int i=0; i<_argc; i++) {
        if(_argv[i]) {
            free(_argv[i]);
            _argv[i] = NULL;
        }
    }

    if(_argv) free(_argv);
}

String HandleCommandLine::toString(int offset)
{
    if(offset < 0 || offset > _argc)
        return (String&) "";

    String ret;
    for(int i=offset; i<_argc; i++)
    {
        if(_argv[i])
        {
            ret.append(_argv[i]);
            ret.append(" ");
        }
    }

    return ret;
}

void HandleCommandLine::setWID()
{
    // needs additional two arguments
    if(_argc > MAX_ARG-2)
        return;

    _argv[_argc++] = strdup("--wid");
    _argv[_argc++] = strdup(CapServerHome::instance()->getCid().c_str());
}

void HandleCommandLine::setVNCPort(int port)
{
    // needs additional two arguments
    if(_argc > MAX_ARG-2)
        return;

    _argv[_argc++] = strdup("-rfbport");
    _argv[_argc++] = strdup(std::to_string(port).c_str());
}

void HandleCommandLine::setDaemon()
{
    // needs additional one arguments
    if(_argc > MAX_ARG-1)
        return;

    _argv[_argc++] = strdup("--daemon");
}

bool setMainProcessServiceHome(const struct cmdConf * conf)
{
    /* get the main service running */
#ifdef __SHAREDT_WIN__
    TCHAR szPath[MAX_PATH];
    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        fprintf(stderr, "Cannot get module file name\n");
        return false;
    }
    ShareDTHome::instance()->set(szPath);
#else
    ShareDTHome::instance()->set(conf->argv[0]);
#endif

    CapServerHome::instance()->setHome(ShareDTHome::instance()->getHome()+String(MAIN_SERVER_PATH), MAINSERVER);

    const String & path = CapServerHome::instance()->getHome();
    if(!fs::exists(path) && !fs::create_directory(path)) {
        std::cerr << "Failed to create working directory: " << path << std::endl;
        return false;
    }

    return true;
}

bool checkMainServiceStarted()
{
    return true;
}

bool setMainServiceFile()
{
    String pathLog = CapServerHome::instance()->getHome() + PATH_SEP_STR + String(MAINSER_LOG);
    LOGGER.setLogFile(pathLog.c_str());
    LOGGER.info() << "Main service log set to: " << pathLog ;

#ifdef __SHAREDT_WIN__
    String pathPid = CapServerHome::instance()->getHome() + PATH_SEP_STR + String(PATH_PID_FILE);
    int curPid = _getpid();
    std::ofstream fs(pathPid.c_str());
    if(!fs)
    {
        LOGGER.error() << "Cannot open pid file: " << pathPid;
        return false;
    }
    fs<< curPid;
    fs.close();

    LOGGER.info() << "Starting server service process: " << curPid;
#else
    // Linux/MacOS will set the pid file later after fork
#endif
    return true;
}

/*
 * Command line to inform service to create child
 * process to run the server procedure
 */
int infoServiceToAction(const char * execCmd)
{
    String alive = ShareDTHome::instance()->getHome() + String(MAIN_SERVER_PATH) + String(PATH_ALIVE_FILE);
    Path aliveReader(alive, std::fstream::in);
    int port;
    try {
        port = aliveReader.readLineAsInt();
        SocketClient sc(LOCALHOST, port);
        sc.SendBytes(execCmd);

        String receive = sc.ReceiveBytes();
        fprintf(stdout, ("%s\n"), receive.c_str() );
    } catch (...) {
        fprintf(stderr, "Failed to connect server process.");
        return RETURN_CODE_INTERNAL_ERROR;
    }

    return RETURN_CODE_SUCCESS;
}