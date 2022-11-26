#ifndef SHAREDT_MAINCONSOLESUBFUNCTION_H
#define SHAREDT_MAINCONSOLESUBFUNCTION_H

extern int mainInform(const struct cmdConf * conf);
extern int mainStart (const struct cmdConf * conf);
extern int mainStop  (const struct cmdConf * conf);
extern int mainRestart (const struct cmdConf * conf);
extern int mainCapture (const struct cmdConf * conf);
extern int mainNewCapture (const struct cmdConf * conf);
extern int mainShow (const struct cmdConf * conf);
extern int noDaemon (const struct cmdConf * conf);
extern int status (const struct cmdConf * conf);
extern int getSc  (const struct cmdConf * conf);


#ifdef __SHAREDT_WIN__
extern int installService (const struct cmdConf * conf);
extern int uninstallService (const struct cmdConf * conf);
extern int startService (const struct cmdConf * conf);
#endif

#endif //SHAREDT_MAINCONSOLESUBFUNCTION_H
