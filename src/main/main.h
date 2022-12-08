#ifndef SHAREDT_MAIN_H
#define SHAREDT_MAIN_H

extern const char * SHAREDT_SERVER_SVCNAME;
extern const char * SHAREDT_SERVER_COMMAND_START;
extern const char * SHAREDT_SERVER_COMMAND_STOP;
extern const char * SHAREDT_SERVER_COMMAND_RESTART;
extern const char * SHAREDT_SERVER_COMMAND_CAPTURE;
extern const char * SHAREDT_SERVER_COMMAND_NEWCAPTURE;
extern const char * SHAREDT_SERVER_COMMAND_SHOW;
extern const char * SHAREDT_SERVER_COMMAND_STATUS;
extern const char * SHAREDT_SERVER_COMMAND_EXPORT;
extern const char * SHAREDT_SERVER_COMMAND_NODAEMON;
extern const char * SHAREDT_SERVER_COMMAND_DISPLAY;
extern const char * SHAREDT_SERVER_COMMAND_CONNECTR;
extern const char * SHAREDT_SERVER_COMMAND_GET;
extern const char * SHAREDT_SERVER_COMMAND_REMOTGET;

struct cmdConf {
    int argc;
    char ** argv;
};
#endif //SHAREDT_MAIN_H
