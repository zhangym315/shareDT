#include "MainService.h"
#include "Path.h"
#include "Logger.h"
#include "MainConsole.h"
#include "CrossPlatform.h"
#include "MainManagementProcess.h"
#include "Foreach.h"
#include "Sock.h"

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>

#ifdef __SHAREDT_WIN__
#include "WindowsProcess.h"

#include <windows.h>
#include <process.h>

//#include <Userenv.h>
#endif

/*
 * Global WID management
 */
WIDMAP _WM;
std::mutex _WMmutex;

/*
 * Stopping all CaptureServer in _WM
 */
static void stopAllSC()
{
    FOREACH(WIDMAP, it, _WM) {
        MainManagementProcess::STATUS statusType = it->second.status();

        if(statusType == MainManagementProcess::STATUS::STARTED) {
            LOGGER.info() << "Sending stopping to WID: " << it->first;
            it->second.send(CAPTURE_STOPPING);

            std::lock_guard<std::mutex> guard(_WMmutex);
            it->second.updateStatus(MainManagementProcess::STATUS::STOPPED);
            LOGGER.info() << "Stopped Capture Server WID: " << it->first;
        }
    }
    return;
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
#ifdef __SHAREDT_WIN__
void HandleCommandSocket(Socket * sk, char * buf)
#else
void HandleCommandSocket(int fd, char * buf)
#endif
{
#ifndef  __SHAREDT_WIN__
    SocketFD sk(fd);
#endif
    String wid;
    StartCapture::CType commandType;
    /* start handle particular --wid specified or start new capture server */
    HandleCommandLine hcl(buf);
    /* parsing input argument */
    hcl.initParsing();
    wid = hcl.getSC().getWID();
    WIDMAP::iterator it = _WM.find(wid);
    commandType = hcl.getSC().getCType();
    String user = hcl.getSC().getUserName();
    const String & capServerHome = hcl.getSC().getCapServerPath();

    /* 1. first check stop specific wid */
    if( commandType == StartCapture::C_STOP ) {
        if(!hcl.hasWid()) {
#ifndef  __SHAREDT_WIN__
            sk.send("command must has a valid \"--wid\" setting");
#else
            sk->send("command must has a valid \"--wid\" setting");
#endif
            return ;
        }

        if(it == _WM.end()) {
            String msg("Can't find status for: ");
            msg.append(wid);
#ifndef  __SHAREDT_WIN__
            sk.send(msg.c_str());
#else
            sk->send(msg.c_str());
#endif
            return;
        }

        LOGGER.info() << "Sending stopping to WID: " << wid;
        it->second.send(CAPTURE_STOPPING);

        std::lock_guard<std::mutex> guard(_WMmutex);
        it->second.updateStatus(MainManagementProcess::STATUS::STOPPED);

        /* send msg back to command line */
        String msg("Capture Server Stopped: ");
        msg.append(wid);

#ifndef  __SHAREDT_WIN__
        sk.send(msg.c_str());
#else
        sk->send(msg.c_str());
#endif

        return;
    }

    /* reconstructing argv if specified by --wid */
    if(!hcl.hasWid())
        hcl.setWID();

    String ret("Starting Capture ID(CID) = ");
    ret.append(wid);
    ret.append("\nStatus: ");

    /* 2. Starting capture server
     * 2.1 First make sure there is no "start" and "started" file under it.
     */
    if( (commandType == StartCapture::C_START || commandType == StartCapture::C_NEWCAPTURE) &&
        (it == _WM.end() || it->second.status() != MainManagementProcess::STATUS::STARTED) ) {
        String start = capServerHome + String(CAPTURE_SERVER_START);
        String started = capServerHome + String(CAPTURE_SERVER_STARTED);
        String alive = capServerHome + PATH_ALIVE_FILE;

        if(fs::exists(start) && !fs::remove(start)){
            LOGGER.error() << "Failed to remove the file: " << start;
        }

        if(fs::exists(started) && !fs::remove(started)){
            LOGGER.error() << "Failed to remove the file: " << started;
        }

        LOGGER.info() << "Starting Capture Server Argument: " << hcl.toString();
        int childPid;


#ifndef __SHAREDT_WIN__
        /* Non-windows, communicate the child process through pipe */
        char ** argv = hcl.getArgv();
        mkfifo(alive.c_str(), 0666);
        if((childPid=fork()) == 0) {
            execv(argv[0], argv);
        }
        ReadWriteFD msg(alive.c_str(), O_RDONLY);
        String answer = msg.read();
#else
        /* windows, communicate the child process through port */
        SocketServer sc(SHAREDT_INTERNAL_PORT_START, 2);
        LOGGER.info() << "Start on port=" << sc.getPort() <<
                    " for communication with CaptureServer=" << hcl.getSC().getWID();
        {
            if(fs::exists(alive) && !fs::remove(alive)){
                String error = "Failed to remove the file: " + alive;
                LOGGER.error() << error;
                sk->send(error.c_str());
                return;
            }
            Path aliveWriter(alive);
            aliveWriter.write(sc.getPort());
        }

        /* create process as the user requested */
        LOGGER.info() << "Retrieving user session token for user=" << user;
        UserSession usrSession(user);
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
                                0,       // No creation flags
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
        LOGGER.info() << "Successuflly create child process, communicating port: " << sc.getPort();
        CloseHandle(pi.hThread);
        childPid = (int) pi.hProcess;

        Socket* s=sc.Accept();
        String answer = s->ReceiveBytes();
        delete s;
#endif
        ret.append(answer);

        LOGGER.info() << "Child process for WID=" << wid <<
                        " started, PID=" << childPid << " CMD=" << hcl.toString();

        /* TODO needs to check if started successfully */

        /* add it to global _WM */
        if(it == _WM.end()) {
            std::lock_guard<std::mutex> guard(_WMmutex);
            _WM.insert(std::pair<String, MainManagementProcess>
                       (wid, MainManagementProcess(alive, MainManagementProcess::STATUS::STARTED)));
        } else {
            std::lock_guard<std::mutex> guard(_WMmutex);
            it->second.updateStatus(MainManagementProcess::STATUS::STARTED);
        }
    } else {
        ret += "Already started";
    }
#ifndef  __SHAREDT_WIN__
    sk.send(ret.c_str());
#else
    sk->send(ret.c_str());
#endif

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

bool setMainProcessServiceHome(const struct cmdConf * conf)
{
    /* get the main service running */
#ifdef __SHAREDT_WIN__
    TCHAR szPath[MAX_PATH];
    if( !GetModuleFileNameA( NULL, szPath, MAX_PATH ) )
    {
        printf("Cannot get module file name\n");
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
