#include "WindowsProvider.h"
#include <chrono>

int WindowsProcessProvider::set(pid_t pid)
{
#if 0
    /* only get the window with a name */
    CapWindow cur = GetWindows(pid, false);
    int ret = cur.size();
    if(ret == 0) return 0;

    _win = make_shared<Windows>(cur);
//    _win = make_shared<Windows>(std::move(cur));
    _pid = pid;

    /* create new thread to monitor */
    createMonitorThread();
    return ret;
#endif
    return 0;
}

/* create new thread to monitor if new windows created by one process */
void WindowsProcessProvider::createMonitorThread()
{
    _wpm = new WindowsProviderChecker(_pid, _win);
    _wpm->setCount();
    _wpm->go();
    _wpm->run();
}

/* TODO decide to implement or not in the futhure */
int WindowsHandlerProvider::set(size_t handler)
{
    return 0;
}

void WindowsProviderChecker::mainImp()
{
#if 0
    while (1)
    {
        std::cerr << "Thread: thread_id: " << _tid << std::endl;
        if(_running)
        {
            CapWindows cur = GetWindows(_monPid, false);
            if(cur.size() != getCount())
                std::cerr << "WindowsProviderChecker::mainImp found new window created or deleted" << std::endl;
        }
        /* sleep for amount */
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
#endif
}
