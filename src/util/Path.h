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

#define SHAREDT_KEYWORD        "ShareDT"
#define CAPTURE_LOG        "ShareDT.log"
#define VNCSERV_LOG        "vncserv.log"
#define MAINSER_LOG        "ShareDTServer.log"
#define FILEPID            "file.pid"
#define SOCKET_FILE        "server.sf"
#define ALIVE_FILE         "alive.mng"
#define MAINSERVER         "MAINSERVER"
#define CAPTURE_SERVER_START    "start"
#define CAPTURE_SERVER_STARTED  "started"
#define CAPTURE_SERVER_STOP     "stop"
#define CAPTURE_SERVER_STOPPED  "stopped"

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
    std::string & getHome();
    const std::string & getArgv0() const;
    const std::string & getArgv0Dir() const;
    bool isValid() const;

  private:
    ShareDTHome();
    static ShareDTHome * _instance;
    std::string _home;
    std::string _execPath; /* argv[0] full path */
    std::string _execDir; /* argv[0] directory */
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
    void setHome(const std::string & path,const std::string & cid);
    const std::string & getHome();
    const std::string & getCid();
    [[nodiscard]] bool  isValid() const { return _valid; }

  private:
    CapServerHome();
    static CapServerHome * _instance;
    std::string _home;
    std::string _cid;
    bool   _valid;
};

static const std::string EMPTY_STRING = "EMPTY_STRING";

class Path
{
  public:
    explicit Path(const std::string& path) : _ffs(path, std::fstream::in | std::fstream::out | std::fstream::app) { }
    Path(const std::string& path, std::ios_base::openmode mode) : _ffs(path, mode) { }
    ~Path() { _ffs.close(); }
    void write(const std::string & data);
    void write(int data);
    void write(char * data);
    int  readLineAsInt();
    std::string readAll();
    static bool checkAndWait(std::string & path, int seconds);

    static void removeContent(const std::string & path);

  private:
    std::fstream _ffs;
};
#endif //_PATH_H_
