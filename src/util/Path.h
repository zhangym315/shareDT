#ifndef _PATH_H_
#define _PATH_H_
#include "TypeDef.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define PATH_SEP_CHAR_UNIX            '/'
#define PATH_SEP_STR_UNIX             "/"
#define PATH_SEP_CHAR_WIN             '\\'
#define PATH_SEP_STR_WIN              "\\"

#ifdef __SHAREDT_WIN__
#  define PATH_SEP_CHAR		PATH_SEP_CHAR_WIN
#  define PATH_SEP_STR		PATH_SEP_STR_WIN
#else
#  define PATH_SEP_CHAR		PATH_SEP_CHAR_UNIX
#  define PATH_SEP_STR		PATH_SEP_STR_UNIX
#endif

#define CAPTURE_LOG        "shareDT.log"
#define VNCSERV_LOG        "vncserv.log"
#define MAINSER_LOG        "shareDT.log"
#define FILEPID            "file.pid"
#define SOCKET_FILE        "server.sf"
#define ALIVE_FILE         "alive.mng"
#define MAINSERVER         "MAINSERVER"

#define PATH_CAPTURE_LOG   PATH_SEP_STR CAPTURE_LOG
#define PATH_VNCSERVER_LOG PATH_SEP_STR VNCSERV_LOG
#define PATH_PID_FILE      PATH_SEP_STR FILEPID
#define PATH_ALIVE_FILE    PATH_SEP_STR ALIVE_FILE

#define VAR_RUN PATH_SEP_STR "var" PATH_SEP_STR "run" PATH_SEP_STR
#define MAIN_SERVER_PATH VAR_RUN MAINSERVER  // $SHAREDTHOME/var/run/MAINSERVER
#define MAIN_SERVER_EXEC PATH_SEP_STR "bin" PATH_SEP_STR START_SERVER_EXEC  // $SHAREDTHOME/bin/${START_SERVER_EXEC}

/*
 * @ShareDTHome
 * Owned by StartServer to provide the APP home
 */
class ShareDTHome {
  public:
    static ShareDTHome * instance() ;

    void   reSet(const char *argv);
    void   set(const char *argv);
    String & getHome();
    bool isValid();

  private:
    ShareDTHome();
    static ShareDTHome * _instance;
    String _home;
    bool   _valid;
};

/*
 * @CapServerHome
 * Singleton instance to provide the Cap Server home
 */
class CapServerHome {
  public:
    static CapServerHome * instance() ;
    void init();
    void setHome(const String & path,const String & cid);
    const String & getHome();
    const String & getCid();

  private:
    CapServerHome();
    static CapServerHome * _instance;
    String _home;
    String _cid;
};
#endif //_PATH_H_