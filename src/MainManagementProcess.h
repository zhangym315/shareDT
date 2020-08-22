#ifndef _MAINMANAGEMENTPROCESS_H_
#define _MAINMANAGEMENTPROCESS_H_

#include "StringTools.h"
#include "ReadWriteFD.h"

#include <map>

class MainManagementProcess;
typedef std::map<String, MainManagementProcess> WIDMAP;

static const char * status[5] = {"started", "stopped", "pending", "UNKNOWN", NULL};

class MainManagementProcess
{
  public:
    enum STATUS {STARTED, STOPPED, PENDING, UNKNOWN};
    MainManagementProcess(const String & alive, STATUS status);
    MainManagementProcess(const String & alive) : MainManagementProcess(alive, UNKNOWN) { }

    void send(const char * buf);

    STATUS status() { return _status; }
    void updateStatus(STATUS status);
  private:
    STATUS _status;
    String _alivePath;
    ReadWriteFD _rw;
};

#endif //_MAINMANAGEMENTPROCESS_H_