#include "SDThread.h"

SDThread::SDThread()
{
    _stopped = false;
    _shutdown = false;
}

void SDThread::stop()
{
    if (_stopped )
        QThread::currentThread()->usleep (100);
    else
        _stopped = true;
}

bool SDThread::isShutDown()
{
    return _shutdown;
}