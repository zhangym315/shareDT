#ifndef _WINDOWPROCESSOR_H_
#define _WINDOWPROCESSOR_H_

#include <iostream>
#include <vector>
#include <mutex>
#include "ImageRect.h"
#include "Thread.h"

enum WINPROCESSOR_RETURN { WINPROCESSOR_RETURN_SUCCESS = 0, WINPROCESSOR_RETURN_ERROR_EXPECTED = 1, WINPROCESSOR_RETURN_ERROR_UNEXPECTED = 2 };
enum SPType { SP_ALLMONITOR, SP_MONITOR, SP_WINDOW, SP_PARTIAL, SP_NULL };

class BaseFrameProcessor {
  public:
    std::unique_ptr<unsigned char[]> ImageBuffer;
    int  imageBufferSize = 0;
    bool firstRun = true;
};

/* Automatically check if new windows has been created */
class WindowVectorProvider : public Thread {
  public:
    /*
     * _pid : -1, show all
     */
    WindowVectorProvider(Pid pid, bool callOnce, bool getAll) : Thread(false),
            _pid(pid), _callOnce(callOnce), _getAll(getAll) { init(); }
    WindowVectorProvider(Pid pid) : WindowVectorProvider(pid, true, false) { }
    /* default constructor show all */
    WindowVectorProvider() : WindowVectorProvider(0, true, false) { }
    ~WindowVectorProvider () { _wins.clear(); }

    void init();
    void mainImp() override;

    const WindowVector & get() { return _wins; }
    void emplace(const CapWindow & win) { _wins.emplace_back(win); }
    void getWinByPid(Pid pid, CapWindow & win) ;
    void getWinByHandler(size_t hd, CapWindow & win);

    WindowVectorProvider & operator=(WindowVectorProvider & other) {
        this->_wins = other.get();
        return *this;
    }

  private:
    /*
     * Method that implements for different platform
     * IOS   is under capture/ios/CGGetWindows.cpp
     * WIN   is under capture/win/...
     * Linux is under capture/linux/...
     */
    void CapGetWindows() ;

    bool         _getAll;   /* Get all windows without name */
    bool         _callOnce;
    Pid          _pid;
    WindowVector _wins;
    std::mutex   _winMtx;
};

class MonitorVectorProvider : public Thread {
  public:
    MonitorVectorProvider(bool callOnce) : Thread(false),
           _callOnce(callOnce) { init(); }
    MonitorVectorProvider(bool callOnce, bool isJoin) : Thread(isJoin),
           _callOnce(callOnce) { init(); }
    MonitorVectorProvider() : MonitorVectorProvider (true) { }
    ~MonitorVectorProvider () { }

    void init();
    void mainImp() override ;

    const MonitorVector & get() { return _mons; }
    std::mutex * getMutex() {
        if(_callOnce) return NULL;
        return &_monMtx;
    }

    void getMonByID(int id, CapMonitor & cap);
    bool getMonByBounds(CapImageRect & bd, CapMonitor & cap);
  private:
    /*
     * Method that implements for different platform
     * IOS   is under capture/ios/CGGetMonitors.cpp
     * WIN   is under capture/win/...
     * Linux is under capture/linux/...
     */
    void CapGetMonitors() ;

    bool          _callOnce;
    MonitorVector _mons ;
    std::mutex    _monMtx;
};

#endif //_WINDOWPROCESSOR_H_
