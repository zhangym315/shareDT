#include <iostream>
#include <string>
#include "Thread.h"

void Thread::go()
{
    _t = std::thread(callMain, this);
    if(_isJoin) _t.join ();
    else _t.detach();
}

THREAD_STARTFUNC_RETURN_DECL Thread::callMain(void *arg)
{
    Thread * instance = static_cast<Thread *>(arg);

    instance->_isRunning = true;
    instance->_tid = get_portable_tid();
    instance->mainImp ();

    return NULL;
}

