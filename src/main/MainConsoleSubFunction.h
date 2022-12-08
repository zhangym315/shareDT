#ifndef SHAREDT_MAINCONSOLESUBFUNCTION_H
#define SHAREDT_MAINCONSOLESUBFUNCTION_H

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

#endif //SHAREDT_MAINCONSOLESUBFUNCTION_H
