/*********************************************************
 * StartServer.cpp                                       *
 *                                                       *
 * main function to start a server                       *
 *                                                       *
 *********************************************************/
#include "StringTools.h"
#include "TypeDef.h"
#include "WindowProcessor.h"
#include "ImageRect.h"
#include "SamplesProvider.h"
#include "StartServer.h"
#include "Daemon.h"
#include "Logger.h"
#include "Enum.h"
#include "MainConsole.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <ctime>
#include <fcntl.h>

#define SERVERNAME "SHAREDT SERVER"
#define MAX_RAND_LENGTH 10

bool _isServerRunning = false;

static const char *SPTypePrefix[] =
{
    "ALLMONITOR"
    , "MONITOR"
    , "WINDOW"
    , "PARTIAL"
    , "NULL"
};

void ReadWriteFDThread::mainImp()
{
    char * inputBuf;
    LOGGER.info() << "Waiting request on: '" << _path << "'";

    while(true) {
        inputBuf = read();

        LOGGER.info() << "Requests: '" << inputBuf << "' received on: '" << _path << "'";
        if(!strcmp(inputBuf, CAPTURE_STOPPING)) {
            _isServerRunning = false;
            LOGGER.info() << "Stopping capture server";
            break;
        }
    }
}

void StartCapture::Usage ()
{
    std::cerr << "--help                       Help message"  << std::endl;
    std::cerr << "\n";
    std::cerr << "The following options related with StartServer option"  << std::endl;
    std::cerr << "-c or --capture win/mon/part Capture method(single windows, monitor or partial)"  << std::endl;
    std::cerr << "-n or --name    capture-name Capture name for window or monitor(only applied to win and mon capture"  << std::endl;
    std::cerr << "                             For window capture (-capture win), name is the handler of win," << std::endl;
    std::cerr << "                             you can find the handler by -s/-showhandler option" << std::endl;
    std::cerr << "-h or --handler handler      The handler for windows capture" << std::endl;
    std::cerr << "-b or --bounds  t,l,r,b      Capture bounds for partial, top left right and bottom" << std::endl;
    std::cerr << "-s [window/win mon/monitor]  Show the window(default) or monitor if [mon/montior] specified " << std::endl;
    std::cerr << "                             If show window, window that doesn't have name would not be printed" << std::endl;
    std::cerr << "-showall                     Show all of the window even without window names" << std::endl;
    std::cerr << "-i or --id ID                For monitor capture, capture the specific monitor id" << std::endl;
    std::cerr << "-p or --process pid          For window capture (-capture win), capture the specific process id's window" << std::endl;
    std::cerr << "                             This option can overwrite -n/-name" << std::endl;
    std::cerr << "--daemon                     Running not in daemon" << std::endl;
    std::cerr << "\n";
    std::cerr << "\n";
    std::cerr << "The following options related with rfb" << std::endl;
    rfbUsage();
}

StartCapture::CType StartCapture::getCType()
{
    return _ctype;
}

void StartCapture::removeAlivePath() const
{
    std::remove(_alivePath.c_str());
    LOGGER.info() << "Removed listening file: " << _alivePath;
}

/*
 * Parse argument
 * Return 0 for success
 * Others for failure
 */
int StartCapture::parseArgs(const vector<String> & args)
{
    /* parse argument that belongs to StartServer */
    for (vector<String>::const_iterator i = args.begin(); i != args.end(); ++i) {
        if (*i == SHAREDT_SERVER_COMMAND_NEWCAPTURE) {
            _ctype = C_NEWCAPTURE;
        } else if (*i == SHAREDT_SERVER_COMMAND_STOP) {
            _ctype = C_STOP;
        } else if (*i == SHAREDT_SERVER_COMMAND_START) {
            _ctype = C_START;
        } else if (*i == SHAREDT_SERVER_COMMAND_RESTART) {
            _ctype = C_RESTART;
        } else if (*i == SHAREDT_SERVER_COMMAND_show) {
            _ctype = C_SHOW;
        }
        else if (*i == "--help") {
            Usage ();
            return RETURN_CODE_INVALID_ARG;
        } else if (*i == "--capture" || *i == "-c" || *i == "--cap") {
            if(_type != SP_NULL) {
                cerr << "You have specified more than one -capture options, try again" << endl;
                return RETURN_CODE_INVALID_ARG;
            }

            /* get the capture type */
            const String & cap = ((i+1) != args.end()) ?  *(++i) : "NULL";
            if(cap == "win" || cap == "window" )
                _type = SP_WINDOW;
            else if (cap == "mon" || cap == "monitor")
                _type = SP_MONITOR;
            else if (cap == "part" || cap == "partial")
                _type = SP_PARTIAL;
            else
                _type = SP_NULL;
        }
        /* get the name of the captured instance */
        else if (*i == "-h" || *i == "--handler") {
            if((i+1) != args.end() && isNumber(*(++i)))
                _hdler = stoi(*i);
            else {
                cerr << "Invalid number found for -h/--handler: " << *i << endl;
                return RETURN_CODE_INVALID_ARG;
            }
        }
        else if (*i == "--show" || *i == "-s") {
            if((i+1) != args.end() &&
                (*(i+1) == "mon" || *(i+1) == "monitor" )) {
                i++;
                _show = S_MONITOR;
            } else {
                if ((i+1) != args.end() &&
                    (*(i+1) == "win" || *(i+1) == "windows" ))
                    i++; /* eat the parameter */
                _show = S_WIN_NAME; /* default to show the windows with name */
            }
        }
        /* get the bounds of the captured instance */
        else if (*i == "--bounds" || *i == "-b") {
            if((i+1) != args.end())
            _name = *(++i);
        }
        /* get the process id that we can check */
        else if (*i == "--process" || *i == "-p") {
            if((i+1) != args.end() && isNumber(*(++i)))
                _pid = stoi(*i);
            else {
                cerr << "Invalid number found for -p/-process: " << *i << endl;
                return RETURN_CODE_INVALID_ARG;
            }
        }
        /* show all of the windows */
        else if (*i == "-showall") {
            _show = S_WIN_ALL;
        }
        /* monitor id */
        else if (*i == "-i" || *i == "--id") {
            if((i+1) != args.end() && isNumber(*(++i)))
                _monID = stoi(*i);
            else {
                cerr << "Invalid number found for -i/--id: " << *i << endl;
                return RETURN_CODE_INVALID_ARG;
            }
        }
        else if(*i == "--daemon" )
        {
            _daemon = true;
        } else if(*i == "--wid" ) {
            if((i+1) != args.end())
                _wID = *(++i);
        }
        else {
        }
    }

    return RETURN_CODE_SUCCESS;
}

bool StartCapture::setWorkingDirectory()
{
    String wPath; // working path
    wPath.append(ShareDTHome::instance()->getHome());
#ifdef __SHAREDT_WIN__
    wPath.append("\\var\\run\\");
#else
    wPath.append("/var/run/");
#endif
    // _wID passed as an argument
    String & cid = (_wID.length() > 0) ? _wID : setAndGetWID();
    wPath.append(cid);
    _capturePath = wPath;
    CapServerHome::instance()->setHome(wPath, cid);

    if(!fs::exists(wPath) && !fs::create_directory(wPath)) {
        std::cerr << "Failed to create working directory: " << wPath << std::endl;
        return false;
    }
    return true;
}

String & StartCapture::setAndGetWID()
{
    _wID.clear ();
    /* prefix */
    _wID.append (ENUM_TO_STR(_type, SPTypePrefix));
    _wID.append ("_");
    _wID.append (std::to_string (std::time (nullptr)));

    if (_type == SP_PARTIAL)
    {
        char s[] = "-";
        _wID.append ("_BD");
        _wID.append (_cap._bounds.toString (s));
    }
    else if (_type == SP_WINDOW) {
        if (_pid != -1) {
            _wID.append ("_PID");
            _wID.append (std::to_string (_pid));
        }
        else {
            _wID.append ("_HID");
            _wID.append (std::to_string (_hdler));
        }
    }
    else if (_type == SP_MONITOR) {
        _wID.append ("_ID");
        _wID.append (std::to_string (_monID));
    }
    else
        _wID.append ("_INVALIDID");

    /* append */
    std::srand ((unsigned int) std::time (NULL));
    String rnd = std::to_string (std::rand ());
    _wID.append ("_RND");
    if (MAX_RAND_LENGTH > rnd.length ())
        _wID.append (std::string (MAX_RAND_LENGTH - rnd.length (), '0') + rnd);
    else
        _wID.append (rnd.substr (0, MAX_RAND_LENGTH));

    return _wID;
}

/*
 * Init variable for daemon mode
 */
void StartCapture::initDaemon()
{
    String captureLog = CapServerHome::instance()->getHome() + PATH_CAPTURE_LOG;
    LOGGER.setLogFile(captureLog.c_str());

#ifndef __SHAREDT_WIN__
    DaemonizeProcess::daemonizeInit();
#endif
}

int StartCapture::initParsing(int argc, char * argv[])
{
    if(argc <= 0)
        return RETURN_CODE_INVALID_ARG;

    /* Argument setting start here */
    vector<String> args(argv + 1, argv + argc);
    String infname, outfname;
    int ret;

    _type = SP_NULL;

    if(parseArgs(args))
        return RETURN_CODE_INVALID_ARG;

    if(_show != S_NONE || _ctype == C_SHOW) {
        show();
        return RETURN_CODE_SUCCESS_SHO;
    }

    if((ret=parseType()) != RETURN_CODE_SUCCESS) return ret;
    /* Argument parsing is done here */

    /* set home path */
    ShareDTHome::instance()->reSet(argv[0]);
    if(!ShareDTHome::instance()->isValid()) {
        LOGGER.error() << "Can't determine HOME PATH for daemon running";
        return RETURN_CODE_INTERNAL_ERROR;
    }

    /* create working directory */
    if(!setWorkingDirectory()) {
        std::cerr << "Failed to create HomePath: " << CapServerHome::instance()->getHome() << std::endl;
        return RETURN_CODE_INTERNAL_ERROR;
    }
    _alivePath = CapServerHome::instance()->getHome() + PATH_ALIVE_FILE;

    return RETURN_CODE_SUCCESS;
}

StartCapture::~StartCapture()
{
}

/*
 * Set parameter to StartCapture
 * Return code:
 *  RETURN_CODE_SUCCESS: set successfully, needs to continue
 *  Others: Can exit program
 */
int StartCapture::init(int argc, char *argv[]) {

    int ret = initParsing(argc, argv);

    if(ret != RETURN_CODE_SUCCESS)
        return ret;

    if(_daemon)
        initDaemon();

    /* create ScreenProvider */
    if (_type == SP_PARTIAL) {
        _sp = new ScreenProviderPartial(_cap._bounds);
    }
    else if (_type == SP_WINDOW) {
        if(_pid == -1)
            _sp = new ScreenProviderWindow(_hdler);
        else
            _sp = new ScreenProviderWindow(_pid);
    }
    else if (_type == SP_MONITOR) {
        _sp = new ScreenProviderMonitor(_monID);
    }
    else if (_type == SP_NULL) {
        Usage();
        return RETURN_CODE_INVALID_ARG;
    }

    /* needs to ensure _sp is valid */
    if(!_sp->isValid()) {
        LOGGER.error() << "Invalid content(monitor ID/partial bounds) specifed for capture";
        return RETURN_CODE_INVALID_ARG;
    }

    /* init rfb server */
    _rfbserver = rfbGetScreen(&argc, argv,
            _sp->getBounds ().getWidth (),
            _sp->getBounds ().getHeight(), 8, 4, 4);
    if (!_rfbserver) {
        LOGGER.error() << "Failed to create RFB server";
        return RETURN_CODE_INVALID_RFB;
    }
    _rfbserver->desktopName = SERVERNAME;

    return RETURN_CODE_SUCCESS;
}  /* end of StartCapture init/constructor */

int  StartCapture::parseType ()
{
    if(_type == SP_PARTIAL && !parseBounds ())
        return RETURN_CODE_INVALID_ARG;
    if(_type == SP_WINDOW && !parseWindows())
        return RETURN_CODE_INVALID_ARG;
    return RETURN_CODE_SUCCESS;
}

/*
 * parse the input into rect
 * Default format should be left,top,right,bottom
 * If the format is not correct, return the default
 *    rect(0, 0, 1024, 1024)
 */
bool StartCapture::parseBounds()
{
    std::istringstream iss( _name );

    String result;
    String token;
    int b[4] = {DEFAULT_BOUNDS_LEFT, DEFAULT_BOUNDS_TOP,
                    DEFAULT_BOUNDS_RIGHT, DEFAULT_BOUNDS_BOTTOM};
    int i = 0;

    while( std::getline( iss, token, ',' ) )
    {
        if(isNumber(token))
            b[i] = stoi(token);
        if (++i >= 4) break;
    }

    /* update rect values */
    _cap._bounds.set(b[0], b[1], b[2], b[3]);

    return true;
}

/*
 * parse Window
 *
 * Return false if win handler can't be found
 *        Or input is NOT int
 */
bool StartCapture::parseWindows ()
{
    int handler;

    if(_hdler) return true;

    /* if process id is valid, set the windows list */
    if (_pid != -1) {
    } else if (!toInt(_name, handler))
        return false;
    else _hdler = handler;

    return true;
}

void StartCapture::show()
{
    if(_show == S_WIN_ALL || _show == S_WIN_NAME || _ctype == C_SHOW)
    {
        /*
         * _pid : -1, show all
         */
        WindowVectorProvider wvp(_pid);
        std::cout << "Windows Lists:" << std::endl;
        for (CapWindow win : wvp.get())
        {
            std::cout << "Handle: " << win.getHandler()
                      << "\tPid: "<< win.getPid ()
                      << "\tWindow name: " << win.getName() << std::endl;
        }

    }
    if (_show == S_MONITOR || _ctype == C_SHOW) {
        MonitorVectorProvider mvp(true);
        std::cout << "Monitor Lists:" << std::endl;
        for (CapMonitor mon : mvp.get())
        {
            std::cout << std::fixed << std::setprecision(3)   /* for scale float cout */
                      << "Name: " << mon.getName()
                      << "\tid: " << mon.getId ()
                      << "\tindex: " << mon.getIndex()
                      << "\toffset: " << mon.getOffset()
                      << "\tsize: "  << mon.getSize ()
                      << "\tOriginalOffset: " << mon.getOrgOffset()
                      << "\tOriginalSize : " << mon.getOrgSize ()
                      << "\tscale: " << mon.getScale() << std::endl;
        }
    }

    return;
}

int StartCapture::getVNCClientCount(struct _rfbClientRec* head) {
    int ret = 0 ;
    struct _rfbClientRec * ptr = head;

    while (ptr && ptr->next != head) {
        ret ++;
        ptr = ptr->next;
    }

    return ret;
}

/*
 * Init RFB server
 * Start capture and send data to client
 **/
void StartCapture::startCaptureServer()
{
    int preConnected = 0;
    int currentConnected = 0;

    if (_sp == NULL)
        LOGGER.error() << "Failed to start server";

    if(!_sp->startSample()) {
        LOGGER.error() << "Failed to start SampleProvider" ;
        return ;
    }

    /* start new thread to get command from MMP(MainManagementProcess)  */
    _listenMMP = new ReadWriteFDThread(_alivePath.c_str(), O_RDONLY);
    _listenMMP->go();
    _isServerRunning = true;

    /* init rfb server to listen on */
    rfbInitServer(_rfbserver);

    while(!_sp->isSampleReady()) {
        std::this_thread::sleep_for(50ms);
    }
    FrameBuffer * fb;

    /* loop through events */
    rfbMarkRectAsModified(_rfbserver, 0, 0,
        _sp->getWidth(), _sp->getHeight());

    LOGGER.info() << "Started startCaptureServer" ;
    while (rfbIsActive(_rfbserver) && _isServerRunning) {
        std::this_thread::sleep_for(50ms);
        rfbProcessEvents(_rfbserver, 10000);

        /*
         * Check connected client
         * If no connected client, pause the SamplesProvider
         * Else, resume SamplesProvider
         */
        currentConnected = getVNCClientCount(_rfbserver->clientHead);
        if(preConnected != currentConnected) {
            LOGGER.info() << "Current connected clients: " << currentConnected ;

            /* new state, no connected clients */
            if(currentConnected == 0 && !_sp->isSamplePaused()) {
                _sp->samplePause ();
            }
            /* new state, start sample capturing */
            else if (preConnected==0 && _sp->isSamplePaused())
                _sp->sampleResume();
            preConnected = currentConnected;
        }
        if(currentConnected == 0) continue;

        /* get frame buffer and sync to clients */
        fb = _sp->getFrameBuffer();
        if(!fb) {
            continue;
        }
        _rfbserver->frameBuffer = (char *) fb->getData();

        rfbMarkRectAsModified(_rfbserver, 0, 0,
            _sp->getWidth(), _sp->getWidth());
    }

    removeAlivePath();
    return;
}
