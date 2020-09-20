#ifndef _MAINMANAGEMENTPROCESS_H_
#define _MAINMANAGEMENTPROCESS_H_

#include "StringTools.h"
#include "ReadWriteFD.h"

#include <map>

class MainManagementProcess;
typedef std::map<String, MainManagementProcess> WIDMAP;

class MainManagementProcess
{
  public:
    enum STATUS {STARTED, STOPPED, PENDING, UNKNOWN};
    MainManagementProcess(const String & alive, const String & home, STATUS status);
    MainManagementProcess(const String & alive, const String & home ) :
                            MainManagementProcess(alive, home, UNKNOWN) { }

    void send(const char * buf);

    STATUS status() const { return _status; }
    void updateStatus(STATUS status);
    void stop();
  private:
    STATUS _status;
    String _alivePath;
    String _home;
    ReadWriteFD _rw;
};

#endif //_MAINMANAGEMENTPROCESS_H_
