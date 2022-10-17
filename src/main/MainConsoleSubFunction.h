#ifndef SHAREDT_MAINCONSOLESUBFUNCTION_H
#define SHAREDT_MAINCONSOLESUBFUNCTION_H

extern int mainInform(const char * command, const struct cmdConf * conf);
extern int mainStart (const char ** cmdArg, const struct cmdConf * conf);
extern int mainStop (const char ** cmdArg, const struct cmdConf * conf);
extern int mainRestart (const char ** cmdArg, const struct cmdConf * conf);
extern int mainCapture (const char ** cmdArg, const struct cmdConf * conf);
extern int mainNewCapture (const char ** cmdArg, const struct cmdConf * conf);
extern int mainShow (const char ** cmdArg, const struct cmdConf * conf);
extern int noDaemon (const char ** cmdArg, const struct cmdConf * conf);
extern int status (const char ** cmdArg, const struct cmdConf * conf);


#ifdef __SHAREDT_WIN__
extern int installService (const char ** cmdArg, const struct cmdConf * conf);
extern int uninstallService (const char ** cmdArg, const struct cmdConf * conf);
extern int startService (const char ** cmdArg, const struct cmdConf * conf);
#endif

#endif //SHAREDT_MAINCONSOLESUBFUNCTION_H
