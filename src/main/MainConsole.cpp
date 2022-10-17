/*********************************************************
 * MainConsole.cpp                                       *
 *                                                       *
 * main function to start a server                       *
 *                                                       *
 *********************************************************/

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
