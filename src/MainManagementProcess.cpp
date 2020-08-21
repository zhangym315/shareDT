#include "MainManagementProcess.h"

#include <fcntl.h>

MainManagementProcess::MainManagementProcess(const String & alive, STATUS status) :
    _alivePath(alive),  _status(status), _rw(alive.c_str(), O_WRONLY)
{
}

void MainManagementProcess::send(const char * buf)
{
    _rw.write(buf);
}

void MainManagementProcess::updateStatus(STATUS status)
{
    _status = status;
}