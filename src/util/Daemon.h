#ifndef _DAEMON_H_
#define _DAEMON_H_


/* singleton class to make process daemon */
class DaemonizeProcess {
  public:
    static DaemonizeProcess * instance();

    void daemonize();

#ifndef __SHAREDT_WIN__
    void daemonizeInit();
#endif

  private:
    DaemonizeProcess();
    static DaemonizeProcess * _instance;

};

#endif //_DAEMON_H_
