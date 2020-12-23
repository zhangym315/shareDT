#ifndef MAINCONSOLE_H
#define MAINCONSOLE_H

#define SHAREDT_SERVER_SVCNAME "shareDTServer"
#define SHAREDT_SERVER_COMMAND_START      "start"
#define SHAREDT_SERVER_COMMAND_STOP       "stop"
#define SHAREDT_SERVER_COMMAND_RESTART    "restart"
#define SHAREDT_SERVER_COMMAND_NEWCAPTURE "newCapture"
#define SHAREDT_SERVER_COMMAND_SHOW       "show"
#define SHAREDT_SERVER_COMMAND_STATUS     "status"
#define SHAREDT_SERVER_COMMAND_EXPORT     "export"


struct cmdConf {
    int argc;
    const char ** argv;
};
#endif
