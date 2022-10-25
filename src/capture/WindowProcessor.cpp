#include <stdio.h>
#include <assert.h>
#include "WindowProcessor.h"

void WindowVectorProvider::init()
{
    /* no need to initialized */
    if(_pid == 0) {
        _wins.clear();
        return ;
    }

    CapGetWindows();
    /*
     * If get once, just no need to create thread
     * to check Windows list perioradically
     */
    if(_callOnce) return;

    /* create new thread and call mainImpl */
    go();
}

/*
 * Running under new thread to check if
 *   new window is created for process _pid
 */
void WindowVectorProvider::mainImp()
{
    while(1) {
        /* sleep for amount */
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        CapGetWindows();
    }
}

void WindowVectorProvider::getWinByPid(pid_t pid, CapWindow & win)
{
    win.clear();
    int offsetX = INT32_MAX;
    int offsetY = INT32_MAX;
    int sizeX  = 0;
    int sizeY = 0;
    for (WindowVector::iterator it = _wins.begin(); it < _wins.end(); it++)
    {
        if(pid == it->getPid()) {
            if(offsetX > it->getOffset().getX()) offsetX = it->getOffset().getX();
            if(offsetY > it->getOffset().getY()) offsetY = it->getOffset().getY();
            if(sizeX  < it->getSize().getX()) sizeX = it->getSize().getX();
            if(sizeY  < it->getSize().getY()) sizeY = it->getSize().getY();

            win = (*it);
            win.push(it->getHandler());
        }
    }

    win.setOffset(offsetX, offsetY);
    win.setSize(sizeX, sizeY);

    win.setWinType(SP_WIN_PROCESS);
}

void WindowVectorProvider::getWinByHandler(size_t hd, CapWindow & win)
{
    win.clear();
    for (WindowVector::iterator it = _wins.begin(); it < _wins.end(); it++)
    {
        if(it->getHandler() == hd) {
            win = *it;
            break;
        }
    }

    win.setWinType(SP_WIN_HANDLER);
}

void MonitorVectorProvider::init()
{

    CapGetMonitors ();

    /*
     * If get once, just no need to create thread
     * to check Windows list perioradically
     */
    if(_callOnce) return;

    /* create new thread and call mainImpl */
    go();
}

/*
 * Running under new thread to check if
 *   monitor is created/adjusted/deleted
 */
void MonitorVectorProvider::mainImp()
{
    while(1) {
        /* sleep for amount */
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        CapGetMonitors ();
    }
}

void MonitorVectorProvider::getMonByID(int id, CapMonitor & cap) {
    for (MonitorVector::iterator it=_mons.begin(); it<_mons.end(); it++ ) {
        if(it->getId() == id) {
            cap = (*it);
            return ;
        }
    }

}

/*
 * Fun: Check which monitor contains the bounds
 *          specified by bd, and
 *      Set bd within the offset for that monitor
 * Ret: true if found,
 */
bool MonitorVectorProvider::getMonByBounds(CapImageRect & bd, CapMonitor & cap) {
    int l = bd.getLT().getX();
    int t = bd.getLT().getY();
    int r = bd.getRB().getX();
    int b = bd.getRB().getY();

    int ml, mt;  /* monitor left and top */
    assert(l <= r);
    assert(t <= b);

    for (MonitorVector::iterator it=_mons.begin(); it<_mons.end(); it++ ) {
        ml = it->getOrgOffset().getX();
        mt = it->getOrgOffset().getY();
        if(l >= ml && t >= mt &&
          r <=  ml + it->getOrgWidth() &&
          b <=  mt + it->getOrgHeight() ) {
            cap = *it;
            /* minus the offset of the monitor */
            bd.set(l-cap.getOrgOffset().getX(), t-cap.getOrgOffset().getY(),
                r-cap.getOrgOffset().getX(), b-cap.getOrgOffset().getY());
            return true;
        }
    }

    return false;
}
