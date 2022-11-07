#ifndef SHAREDT_SHAREDT_H
#define SHAREDT_SHAREDT_H

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
extern const char * SHAREDT_SERVER_COMMAND_GET;
extern const char * SHAREDT_SERVER_COMMAND_REMOTGET;

struct cmdConf {
    int argc;
    const char ** argv;
};
#endif //SHAREDT_SHAREDT_H
