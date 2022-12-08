#include "StringTools.h"
#include "TypeDef.h"
#include "WindowProcessor.h"
#include "ImageRect.h"
#include "SamplesProvider.h"
#include "Capture.h"
#include "Daemon.h"
#include "Logger.h"
#include "Enum.h"
#include "main/main.h"
#include "InputInterface.h"
#include "CaptureInfo.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <ctime>
#include <fcntl.h>

#ifdef __SHAREDT_WIN__
#include <direct.h>
#endif

#define SERVERNAME "SHAREDT SERVER"
#define MAX_RAND_LENGTH 10

static const char *SPTypePrefix[] =
{
    "ALLMONITOR"
    , "MONITOR"
    , "WINDOW"
    , "PARTIAL"
    , "NULL"
};

void CommandChecker::mainImp()
{
    std::string stop    = _path + PATH_SEP_STR + CAPTURE_SERVER_STOP;
    std::string stopped = _path + PATH_SEP_STR + CAPTURE_SERVER_STOPPED;

    while(!fs::exists(stop))
    {
        std::this_thread::sleep_for(1s);
    }

    fs::remove(stop);
    std::ofstream ofs(stopped.c_str());
    ofs << "stopped";
    ofs.close();

    LOGGER.info() << "Stopping CaptureServer request recieved";
    CaptureInfo::instance()->setIsRunning(false);
}

void Capture::Usage ()
{
    std::string instanceName = _ctype == C_LOCALDISPLAYER ? "LocalDisplayer" : "ShareDTServer";
    std::cerr << "--help                       Help message"  << std::endl;
    std::cerr << "\n";
    std::cerr << "The following options related with "<< instanceName << " option"  << std::endl;
    std::cerr << "-c or --capture win/mon/part Capture method(single windows, monitor or partial)"  << std::endl;
    std::cerr << "-n or --name    capture-name Capture name for window or monitor(only applied to win and mon capture"  << std::endl;
    std::cerr << "                             For window capture (-capture win), name is the handler of win," << std::endl;
    std::cerr << "                             you can find the handler by -s/-showhandler option" << std::endl;
    std::cerr << "-h or --handler handler      The handler for windows capture" << std::endl;
    std::cerr << "-b or --bounds  t,l,r,b      Capture bounds for partial, top left right and bottom" << std::endl;
    std::cerr << "-i or --id ID                For monitor capture, capture the specific monitor id" << std::endl;
    std::cerr << "-p or --process pid          For window capture (-capture win), capture the specific process id's window" << std::endl;
    std::cerr << "                             This option can overwrite -n/-name" << std::endl;
    std::cerr << "-s [window/win mon/monitor]  Show the window(default) or monitor if [mon/montior] specified " << std::endl;
    std::cerr << "                             If show window, window that doesn't have name would not be printed" << std::endl;
    std::cerr << "--showall                    Show all of the window even without window names" << std::endl;
if (_ctype != C_LOCALDISPLAYER) {
    std::cerr << "--daemon                     Running in daemon mode" << std::endl;
    std::cerr << "--wid                        Specify the working id (wid) of the capture server. If not" << std::endl;
    std::cerr << "                             sepcified, ShareDTServer will generate a random wid for it." << std::endl;
}
    std::cerr << std::endl;
}

Capture::CType Capture::getCType()
{
    return _ctype;
}

void Capture::removeAlivePath() const
{
    std::remove(_alivePath.c_str());
    LOGGER.info() << "Removed listening file: " << _alivePath;
}

/*
 * Strip quote, retrieve values and set to "value"
 * Count of paremeters that going forward is returned
 */
static int getValueWithQuote(vector<std::string>::const_iterator start,
                              vector<std::string>::const_iterator end,
                              std::string & value)
{
    const std::string & tmp = *(++start);
    int i = 1;

    if(tmp.front() == '"')
    {
        if(tmp.back() == '"')
        {
            /* skip the first and last quote character */
            value = tmp.substr(1, tmp.length()-2);
        } else
        {
            value = tmp.substr(1);
            while ((start + 1) != end)
            {
                i++;
                const std::string & tmp1 = *(++start);
                value.append(" ");
                if(tmp1.back() == '"')
                {
                    value.append(tmp1.substr(0, tmp1.length()-1));
                    break;
                } else
                {
                    value.append(tmp1);
                }
            }
        }
    } else
    {
        value = tmp;
    }

    return i;
}

/*
 * Parse argument
 * Return 0 for success
 * Others for failure
 */
int Capture::parseArgs(const vector<std::string> & args)
{
    /* parse argument that belongs to StartServer */
    for (auto i = args.begin(); i != args.end(); ++i) {

        if (_ctype == C_NONE) {
            if (*i == SHAREDT_SERVER_COMMAND_CAPTURE) {
                _ctype = C_CAPTURE;
            } else if (*i == SHAREDT_SERVER_COMMAND_NEWCAPTURE) {
                _ctype = C_NEWCAPTURE;
            } else if (*i == SHAREDT_SERVER_COMMAND_STOP) {
                if((i+1) != args.end() && *(i+1) == "all")
                    _ctype = C_STOP_ALL_SC;
                else
                    _ctype = C_STOP;
            } else if (*i == SHAREDT_SERVER_COMMAND_START) {
                _ctype = C_START;
            } else if (*i == SHAREDT_SERVER_COMMAND_RESTART) {
                _ctype = C_RESTART;
            } else if (*i == SHAREDT_SERVER_COMMAND_SHOW) {
                _ctype = C_SHOW;
            } else if (*i == SHAREDT_SERVER_COMMAND_STATUS) {
                _ctype = C_STATUS;
            } else if (*i == SHAREDT_SERVER_COMMAND_EXPORT) {
                _ctype = C_EXPORT;
            } else if (*i == SHAREDT_SERVER_COMMAND_REMOTGET) {
                _ctype = C_REMOTEGET;
            }

            if (_ctype != C_NONE) continue;
        }

        if (*i == "--help") {
            Usage ();
            rfbUsagePrint();
            return RETURN_CODE_INVALID_ARG;
        } else if (*i == "--capture" || *i == "-c" || *i == "--cap") {
            if(_type != SP_NULL) {
                cerr << "You have specified more than one -capture options, try again" << endl;
                return RETURN_CODE_INVALID_ARG;
            }

            /* get the capture type */
            const std::string & cap = ((i+1) != args.end()) ?  *(++i) : "NULL";
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
            } else if ((i+1) != args.end() &&
                    (*(i+1) == "win" || *(i+1) == "windows" )) {
                    i++; /* eat the parameter */
                    _show = S_WIN_ALL;
            } else {
                _show = S_ALL; /* default to show the windows with name */
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
        else if (*i == "--showall") {
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
        } else if (*i == "--username") {
            if((i+1) != args.end())
            {
                i += getValueWithQuote(i, args.end(), _user);
            }
        } else if (*i == "-rfbport") {
            if((i+1) != args.end())
            {
                _vncPort = std::atoi((++i)->c_str());
            }
        } else if ( (*i) == "--frequency" ) {
            _frequency = stoi(*(++i));
            if ( _frequency < 0 || _frequency > 1000 ) {
                std::cerr << "Invalid value for --frequency: " << _frequency << std::endl;
                return RETURN_CODE_INVALID_ARG;
            }
        }
        else {
            /* parameter without options should be host(:port) */
            std::string s = *i;
            auto pos = s.find(':');
            if (pos != std::string::npos ) {
                _host = s.substr(0, pos);
                toInt(s.substr(pos + 1, s.length()), _vncPort);
            } else {
                _host = s;
            }

            _unrecognizedOptions.emplace_back(*i);
        }
    }

    return RETURN_CODE_SUCCESS;
}

bool Capture::setWorkingDirectory()
{
    std::string wPath; // working path
    wPath.append(ShareDTHome::instance()->getHome());
#ifdef __SHAREDT_WIN__
    wPath.append("\\var\\run\\");
#else
    wPath.append("/var/run/");
#endif

    // _wID passed as an argument
    std::string & cid = (_wID.length() > 0) ? _wID : setAndGetWID();
    wPath.append(cid);
    _capturePath = wPath;
    CapServerHome::instance()->setHome(wPath, cid);

    if(!fs::exists(wPath) && !fs::create_directory(wPath)) {
        std::cerr << "Failed to create working directory: " << wPath << std::endl;
        return false;
    }
    return true;
}

std::string & Capture::setAndGetWID()
{
    _wID.clear ();

    if ( _ctype == Capture::C_EXPORT ) {
        _wID.append ("EXPORT_");
    }

    _wID.append (ENUM_TO_STR(_type, SPTypePrefix) +
                 "_" + std::to_string (std::time (nullptr)));

    if (_type == SP_PARTIAL)
    {
        char s[] = "-";
        _wID.append ("_BD" + _cap._bounds.toString (s));
    }
    else if (_type == SP_WINDOW) {
        if (_pid != -1) {
            _wID.append ("_PID" + std::to_string (_pid));
        }
        else {
            _wID.append ("_HID" + std::to_string (_hdler));
        }
    }
    else if (_type == SP_MONITOR) {
        _wID.append ("_ID" + std::to_string (_monID));
    }
    else
        _wID.append ("_INVALIDID");

    /* append */
    std::srand ((unsigned int) std::time (NULL));
    std::string rnd = std::to_string (std::rand ());
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
void Capture::initDaemon()
{
    std::string captureLog = CapServerHome::instance()->getHome() + PATH_CAPTURE_LOG;
    LOGGER.setLogFile(captureLog.c_str());

    LOGGER.info() << "Set CaptureServer log to file=" << captureLog;
    LOGGER.info() << "Starting CaptureServer" ;

#ifndef __SHAREDT_WIN__
    DaemonizeProcess::daemonizeInit();
#else
    /* Change the working directory  */
    const char * cwd = CapServerHome::instance()->getHome().c_str();
    _chdir(cwd);

    char capVncLog[MAX_PATH];
    memset(capVncLog, 0, MAX_PATH);
    strcat(capVncLog, cwd);
    strcat(capVncLog, PATH_VNCSERVER_LOG);

    freopen(capVncLog, "a", stdout);
    freopen(capVncLog, "a", stderr);

#endif
}

int Capture::initParsing(int argc, char * argv[])
{
    if(argc <= 0)
        return RETURN_CODE_INVALID_ARG;

    /* Argument setting start here */
    vector<std::string> args(argv + 1, argv + argc);
    std::string infname, outfname;
    int ret;

    _type = SP_NULL;

    if(parseArgs(args))
        return RETURN_CODE_INVALID_ARG;

    if(_show != S_NONE || _ctype == C_SHOW) {
        show();
        return RETURN_CODE_SUCCESS_SHO;
    }

    /* Argument parsing is done here */
    if((ret=parseType()) != RETURN_CODE_SUCCESS) return ret;

    /* no need to setup HomePath for localDisplayer */
    if (_ctype == C_LOCALDISPLAYER || _ctype == C_REMOTEGET) return RETURN_CODE_SUCCESS;

    if (_ctype == C_NONE) return RETURN_CODE_INVALID_ARG;

    /* set home path */
    ShareDTHome::instance()->reSet(argv[0]);
    if(!ShareDTHome::instance()->isValid()) {
        LOGGER.error() << "Can't determine HOME PATH for daemon running";
        return RETURN_CODE_INTERNAL_ERROR;
    }

    /* create working directory */
    try {
        if(!setWorkingDirectory()) {
            std::cerr << "Failed to create HomePath: " << CapServerHome::instance()->getHome() << std::endl;
            return RETURN_CODE_INTERNAL_ERROR;
        }
        _alivePath = CapServerHome::instance()->getHome() + PATH_ALIVE_FILE;
    } catch (...) {
        LOGGER.error() << "Failed to create working directory at HomePath=" << CapServerHome::instance()->getHome();
        return RETURN_CODE_INTERNAL_ERROR;
    }

    return RETURN_CODE_SUCCESS;
}

Capture::~Capture()
{
//    delete _sp;
}

/*
 * Set parameter to Capture
 * Return code:
 *  RETURN_CODE_SUCCESS: set successfully, needs to continue
 *  Others: Can exit program
 */
int Capture::initSrceenProvider()
{
    if( _daemon )
        initDaemon();

    /*
     * Create ScreenProvider
     * The new ScreenProvider will also initialize
     *    SampleProvider and FrameGetterSystem
     */
    if (_type == SP_PARTIAL) {
        _sp = new ScreenProviderPartial(_cap._bounds, _frequency);
    }
    else if (_type == SP_WINDOW) {
        if(_pid == -1)
            _sp = new ScreenProviderWindow(_hdler, _frequency);
        else
            _sp = new ScreenProviderWindow(_pid, _frequency);
        _name = ((ScreenProviderWindow * )_sp)->get().getName();
    }
    else if (_type == SP_MONITOR) {
        _sp = new ScreenProviderMonitor(_monID, _frequency);
        _name = ((ScreenProviderMonitor * )_sp)->get().getName();
    }
    else if (_ctype == C_LOCALDISPLAYER && _type == SP_NULL) {
        _type = SP_MONITOR;
        _sp = new ScreenProviderMonitor(_frequency);
        _name = ((ScreenProviderMonitor * )_sp)->get().getName();
    }
    else if (_type == SP_NULL) {
        Usage();
        rfbUsagePrint();
        return RETURN_CODE_INVALID_ARG;
    }

    /* needs to ensure _sp is valid */
    if(!_sp->isValid()) {
        LOGGER.error() << "Invalid content(monitor ID/partial bounds) specifed for capture";
        return RETURN_CODE_CANNOT_PARSE;
    }

    return RETURN_CODE_SUCCESS;
}  /* end of Capture init/constructor */

int  Capture::parseType ()
{
    if ((_type == SP_PARTIAL && !parseBounds()) ||
        (_type == SP_WINDOW && !parseWindows()))
        return RETURN_CODE_INVALID_ARG;

    return RETURN_CODE_SUCCESS;
}

/*
 * parse the input into rect
 * Default format should be left,top,right,bottom
 * If the format is not correct, return the default
 *    rect(0, 0, 1024, 1024)
 */
bool Capture::parseBounds()
{
    std::istringstream iss( _name );

    std::string result;
    std::string token;
    int b[4] = {DEFAULT_BOUNDS_LEFT, DEFAULT_BOUNDS_TOP,
                    DEFAULT_BOUNDS_RIGHT, DEFAULT_BOUNDS_BOTTOM};
    int i = 0;

    while( std::getline( iss, token, ',' ) )
    {
        if(isNumber(token))
            b[i] = stoi(token);
        else return false;

        if (++i >= 4) return false;
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
bool Capture::parseWindows ()
{
    int handler;

    if(_hdler) return true;

    /* if process id is valid, set the windows list */
    if (_pid != -1)
    {
    } else if (!toInt(_name, handler))
        return false;
    else _hdler = handler;

    return true;
}

void Capture::show()
{
    if(_show == S_WIN_ALL || _show == S_WIN_NAME || _show == S_ALL || _ctype == C_SHOW)
    {
        /*
         * _pid : -1, show all
         */
        WindowVectorProvider wvp(_pid);
        std::cout << "Windows Lists:" << std::endl;
        for (const CapWindow& win : wvp.get())
        {
            std::cout << "Handle: " << win.getHandler()
                      << "\tPid: "<< win.getPid ()
                      << "\tWindow name: " << win.getName() << std::endl;
        }

    }

    if (_show == S_MONITOR || _show == S_ALL || _ctype == C_SHOW)
    {
        MonitorVectorProvider mvp(true);
        std::cout << "\nMonitor Lists:" << std::endl;
        for (const CapMonitor& mon : mvp.get())
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
}
