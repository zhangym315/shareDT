#ifndef _MAINMANAGEMENTPROCESS_H_
#define _MAINMANAGEMENTPROCESS_H_

#include "StringTools.h"
#include "ReadWriteFD.h"

#include <map>

#define VNCSERVER_PORT_START 5900
class MainManagementProcess;
typedef std::map<std::string, MainManagementProcess> WIDMAP;

class MainManagementProcess
{
  public:
    enum STATUS {STARTED, STOPPED, PENDING, UNKNOWN};
    MainManagementProcess(const std::string & alive, const std::string & home, int port, STATUS status);
    MainManagementProcess(const std::string & alive, const std::string & home, int port ) :
                            MainManagementProcess(alive, home, port, UNKNOWN) { }

    void send(const char * buf);

    STATUS status() const { return _status; }
    void updateStatus(STATUS status);
    void stop();

    int getPort() const { return _vncport; }
  private:
    STATUS _status;
    std::string _alivePath;
    std::string _home;
    ReadWriteFD _rw;
    int    _vncport;
};

#endif //_MAINMANAGEMENTPROCESS_H_
