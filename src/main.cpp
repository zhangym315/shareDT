/*
 * ShareDT main program
 */
#include "MainGUI.h"
#include "LocalDisplayer.h"
#include "SubFunction.h"
#include "ExportImages.h"

#ifdef __SHAREDT_WIN__
#include <tchar.h>
#include <strsafe.h>

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
        { "service",    &startService },     /* from scm service      */
        { "uninstall",  &uninstallService }  /* uninstall service     */
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
    struct cmdConf cconf{};
    cconf.argc = argc;
    cconf.argv = argv;

    // no parameter, start GUI
    if (argc == 1) return mainGUI(&cconf);

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

