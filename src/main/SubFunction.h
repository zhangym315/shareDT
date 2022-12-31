#ifndef SHAREDT_SUBFUNCTION_H
#define SHAREDT_SUBFUNCTION_H

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
extern const char * SHAREDT_SERVER_COMMAND_CONNECT;
extern const char * SHAREDT_SERVER_COMMAND_GET;
extern const char * SHAREDT_SERVER_COMMAND_REMOTGET;

struct cmdConf {
    int argc;
    char ** argv;
};

extern int mainStart (struct cmdConf * conf);
extern int mainStop  (struct cmdConf * conf);
extern int mainRestart (struct cmdConf * conf);
extern int mainCapture (struct cmdConf * conf);
extern int mainNewCapture (struct cmdConf * conf);
extern int mainShow (struct cmdConf * conf);
extern int noDaemon (struct cmdConf * conf);
extern int status (struct cmdConf * conf);
extern int getSc  (struct cmdConf * conf);
extern int connectRemote  (struct cmdConf * conf);

#ifdef __SHAREDT_WIN__
extern int installService (struct cmdConf * conf);
extern int uninstallService (struct cmdConf * conf);
extern int startService (struct cmdConf * conf);
#endif

#endif //SHAREDT_SUBFUNCTION_H
