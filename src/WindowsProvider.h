#ifndef _WINDOWSPROVIDER_H_
#define _WINDOWSPROVIDER_H_

#include <iostream>
#include "Thread.h"
#include "ImageRect.h"

using namespace std;

typedef std::vector<CapWindow> Windows;
typedef std::vector<CapMonitor> Monitors;

class WindowsProviderChecker : public Thread
{
  public:
    WindowsProviderChecker(Pid pid, shared_ptr<Windows> win) :
            Thread(false), _running(false), _monPid(pid), _monWin(win) {
    }
    WindowsProviderChecker() :
            Thread(false), _running(false), _monPid(-1), _monWin(NULL) { }
    void go() { Thread::go(); }
    void mainImp() final ;

    void pause() { _running = false; }
    void run()   { _running = true; }
    void setCount() {
        if(!_monWin) {
            _winCount = _monWin->size();
        }
        else _winCount = 0;
    }
    int getCount() {
        return _winCount;
    }

  private:
    bool _running;
    Pid  _monPid;
    shared_ptr<Windows> _monWin;
    int  _winCount;        /* current window number monitored */

};

class WindowsProvider
{
  public:
    WindowsProvider()  { _win = make_shared<Windows>();}
    ~WindowsProvider() { }

    virtual shared_ptr<Windows> & get() { return _win; }

  protected:
    shared_ptr<Windows> _win;
};

class WindowsProcessProvider : public WindowsProvider
{
  public:
    WindowsProcessProvider() : WindowsProcessProvider(-1) { }
    WindowsProcessProvider(Pid pid) : _pid(pid), _wpm(NULL) { }
    int set(Pid pid);
    void    createMonitorThread();

  private:
    Pid                      _pid;
    WindowsProviderChecker * _wpm;
};

class WindowsHandlerProvider : public WindowsProvider
{
  public:
    WindowsHandlerProvider() : WindowsHandlerProvider(0) { }
    WindowsHandlerProvider(size_t handler) : _handler(handler) { }
    int set(size_t handler);

  private:
    size_t  _handler;
};

#endif //_WINDOWSPROVIDER_H_
