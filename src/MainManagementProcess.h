#ifndef _MAINMANAGEMENTPROCESS_H_
#define _MAINMANAGEMENTPROCESS_H_

#include "StringTools.h"
#include "ReadWriteFD.h"

#include <map>

#define VNCSERVER_PORT_START 5900
class MainManagementProcess;
typedef std::map<String, MainManagementProcess> WIDMAP;

class MainManagementProcess
{
  public:
    enum STATUS {STARTED, STOPPED, PENDING, UNKNOWN};
    MainManagementProcess(const String & alive, const String & home, int port, STATUS status);
    MainManagementProcess(const String & alive, const String & home, int port ) :
                            MainManagementProcess(alive, home, port, UNKNOWN) { }

    void send(const char * buf);

    STATUS status() const { return _status; }
    void updateStatus(STATUS status);
    void stop();

    int getPort() const { return _vncport; }
  private:
    STATUS _status;
    String _alivePath;
    String _home;
    ReadWriteFD _rw;
    int    _vncport;
};

#endif //_MAINMANAGEMENTPROCESS_H_
