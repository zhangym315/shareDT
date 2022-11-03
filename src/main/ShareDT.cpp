/*
 * ShareDT
 * Show all of the windows/monitors that can be shared
 */
#include <QApplication>
#include <QImage>
#include <QDesktopWidget>

#include "ShareDT.h"
#include "ShareDTWindow.h"

#ifdef __SHAREDT_WIN__
#include <Shlobj.h>
#include <windows.h>

// Hide console for windows
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#endif

static void initShareDT(const char * argv0)
{
    ShareDTHome::instance()->set(argv0);
#ifdef __SHAREDT_WIN__
    WCHAR * filepath;
    if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &filepath))) {
    }
    std::wstring ws(filepath);
    String varRun = String(ws.begin(), ws.end()) + String(PATH_SEP_STR) +
                    String(SHAREDT_KEYWORD) + String(PATH_SEP_STR) +  String(VAR_RUN);
    if (!fs::exists(varRun)) fs::create_directories(varRun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((varRun+String(CAPTURE_LOG)).c_str());
#else
    String varrun = ShareDTHome::instance()->getHome() + String(VAR_RUN);
    if (!fs::exists(varrun)) fs::create_directories(varrun);
    //set log file to var/run/ShareDT.log
    Logger::instance().setLogFile((ShareDTHome::instance()->getHome() +
                                   String(VAR_RUN)+String(CAPTURE_LOG)).c_str());
#endif
}

static int localDisplayer(const char ** a, const struct cmdConf * conf)
{
    QApplication app(const_cast<int&> (conf->argc), const_cast<char **>(conf->argv));
    LocalDisplayer gui(const_cast<int&> (conf->argc), const_cast<char **> (conf->argv));

    if (!gui.isInited()) {
        return -1;
    }

    gui.show();
    gui.startFetcher();
    return QApplication::exec();
}
#include <fcntl.h>
#ifdef __SHAREDT_WIN__
#include <tchar.h>
#include <strsafe.h>
#else
#include "Daemon.h"
#endif

#include "MainConsoleSubFunction.h"
#include "ExportImages.h"

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
    if (argc == 1) {
        initShareDT(argv[0]);
        QApplication app(argc, argv);
        LOGGER.info() << "Starting " << argv[0] << " ...";

        ShareDTWindow gui(argc, argv);
        gui.show();
        return QApplication::exec();
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
            { "export",     &mainExport  },     /* cli to export images  */
            { "display",    &localDisplayer}      /* cli to export images  */
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

