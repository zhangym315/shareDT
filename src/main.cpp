/*
 * ShareDT main program
 */
#include <QApplication>
#include <QImage>
#include <QDesktopWidget>

#include "main.h"
#include "MainGUI.h"
#include "LocalDisplayer.h"

#ifdef __SHAREDT_WIN__
#include <tchar.h>
#include <strsafe.h>
#else
#include "Daemon.h"
#endif

#include "main/MainConsoleSubFunction.h"
#include "ExportImages.h"

#ifdef __SHAREDT_WIN__
#include <Shlobj.h>
#include <windows.h>

// Hide console for windows
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

/*
 * All sub command, without sub command(argc==1), it will start the main GUI.
 */
const char * SHAREDT_SERVER_SVCNAME            = "shareDTServer";
const char * SHAREDT_SERVER_COMMAND_START      = "start";
const char * SHAREDT_SERVER_COMMAND_STOP       = "stop";
const char * SHAREDT_SERVER_COMMAND_RESTART    = "restart";
const char * SHAREDT_SERVER_COMMAND_CAPTURE    = "capture";
const char * SHAREDT_SERVER_COMMAND_NEWCAPTURE = "newCapture";
const char * SHAREDT_SERVER_COMMAND_SHOW       = "show";
const char * SHAREDT_SERVER_COMMAND_STATUS     = "status";
const char * SHAREDT_SERVER_COMMAND_EXPORT     = "export";
const char * SHAREDT_SERVER_COMMAND_NODAEMON   = "nodaemon";
const char * SHAREDT_SERVER_COMMAND_DISPLAY    = "display";
const char * SHAREDT_SERVER_COMMAND_CONNECTR   = "connect";
const char * SHAREDT_SERVER_COMMAND_GET        = "get";
const char * SHAREDT_SERVER_COMMAND_REMOTGET   = "remoteGet";

/*
 * Init main GUI
 */
static void initShareDT(const char * argv0)
{
    ShareDTHome::instance()->set(argv0);
#ifdef __SHAREDT_WIN__
    WCHAR * filepath;
    if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &filepath))) {
    }
    std::wstring ws(filepath);
    std::string varRun = std::string(ws.begin(), ws.end()) + std::string(PATH_SEP_STR) +
                    std::string(SHAREDT_KEYWORD) + std::string(PATH_SEP_STR) +  std::string(VAR_RUN);
    if (!fs::exists(varRun)) fs::create_directories(varRun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((varRun+std::string(CAPTURE_LOG)).c_str());
#else
    std::string varrun = ShareDTHome::instance()->getHome() + std::string(VAR_RUN);
    if (!fs::exists(varrun)) fs::create_directories(varrun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((ShareDTHome::instance()->getHome() +
                                   std::string(VAR_RUN)+std::string(CAPTURE_LOG)).c_str());
#endif
}


static const struct {
    const char *name;
    int (*func)(struct cmdConf *cconf);
} cmdHandlers[] = {
        { SHAREDT_SERVER_COMMAND_START ,     &mainStart   },     /* start service          */
        { SHAREDT_SERVER_COMMAND_STOP  ,     &mainStop    },     /* stop  service          */
        { SHAREDT_SERVER_COMMAND_RESTART,    &mainRestart },     /* restart service        */
        { SHAREDT_SERVER_COMMAND_CAPTURE,    &mainCapture },     /* capture command        */
        { SHAREDT_SERVER_COMMAND_NEWCAPTURE, &mainNewCapture },  /* new capture process    */
        { SHAREDT_SERVER_COMMAND_SHOW,       &mainShow    },     /* command show win       */
        { SHAREDT_SERVER_COMMAND_NODAEMON,   &noDaemon    },     /* run in no daemon mode  */
        { SHAREDT_SERVER_COMMAND_STATUS,     &status      },     /* status of current pro  */
        { SHAREDT_SERVER_COMMAND_EXPORT,     &mainExport  },     /* export images          */
        { SHAREDT_SERVER_COMMAND_DISPLAY,    &localDisplayer},   /* local display          */
        { SHAREDT_SERVER_COMMAND_CONNECTR,   &connectRemote},    /* connect remote display */
        { SHAREDT_SERVER_COMMAND_GET,        &getSc }            /* Get screen             */
#ifdef  __SHAREDT_WIN__
        ,{ "install",    &installService },  /* install service       */
        { "service",    &startService },    /* from scm service      */
        { "uninstall",  &uninstallService } /* uninstall service     */
#endif
};

static void Usage()
{
    fprintf(stdout, "%s\n",
            "Usage: ShareDT <subcommand> ");

    for (const auto & e : cmdHandlers) {
        fprintf(stdout, "               %s\n", e.name);
    }
}

int main(int argc, char** argv)
{
    // no parameter, start GUI
    if (argc == 1) {
        initShareDT(argv[0]);
        QApplication app(argc, argv);
        LOGGER.info() << "Starting " << argv[0] << " ...";

        ShareDTWindow gui(argc, argv);
        gui.show();
        return QApplication::exec();
    }

    struct cmdConf cconf{};
    cconf.argc = argc;
    cconf.argv = argv;

    for (const auto & cmdHandler : cmdHandlers) {
        if (chars_equal(cmdHandler.name, argv[1])) {
            int ret = cmdHandler.func(&cconf);
            fflush(stdout);
            return ret;
        }
    }
    Usage();
    return -1;
}

