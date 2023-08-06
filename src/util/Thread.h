#ifndef _THREAD_LOCAL_H_
#define _THREAD_LOCAL_H_

#include <thread>
#include "TypeDef.h"
#include "Pid.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#endif

class Thread : public std::thread {
  public:
    Thread(bool isjoin) : _isJoin(isjoin), _isRunning(false) { }
    Thread() : Thread(true) { }
    ~Thread() { if(_t.joinable()) _t.join(); }

    /* To create a thread and run the mainImp function */
    virtual void go();

  protected:
    static THREAD_STARTFUNC_RETURN_DECL callMain(void *arg);
    virtual void mainImp() = 0;

    portable_tid_t  _tid;
    std::thread     _t;
    bool            _isJoin;   /* join or detach */
    bool            _isRunning;
};

#endif //_THREAD_LOCAL_H_
