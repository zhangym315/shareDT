#include "Capture.h"
#include "MainManagementProcess.h"

#include <fcntl.h>

MainManagementProcess::MainManagementProcess(const std::string & alive, const std::string & home,
                                             int port, STATUS status) :
                _alivePath(alive),  _status(status), _vncport(port),
                _rw(alive.c_str(), O_WRONLY), _home(home)
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

void MainManagementProcess::stop()
{
    std::string stop    = _home + PATH_SEP_STR + CAPTURE_SERVER_STOP;
    std::string stopped = _home + PATH_SEP_STR + CAPTURE_SERVER_STOPPED;

    if(fs::exists(stop) && !fs::remove(stop)){
        LOGGER.error() << "Failed to remove the stop file: " << stop;
    }
    if(fs::exists(stopped) && !fs::remove(stopped)){
        LOGGER.error() << "Failed to remove the stopped file: " << stopped;
    }

    std::ofstream ofs(stop.c_str());
    ofs << "stop";
    ofs.close();

    int i;
    /* wait for 20s for CaptureServer to stop */
    for (i = 0 ; i < 20; i++)
    {
        if(fs::exists(stopped))
            break;
        this_thread::sleep_for(1s);
    }

    /* failed to know the CaptureServer */
    if(i == 20)
    {
        LOGGER.error() << "Waited 20s for CaptureServer to stop, but no responds for wid=" << _home;
        return;
    }
}