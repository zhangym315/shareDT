#include "ScreenProvider.h"
#include "SamplesProvider.h"
#include "Enum.h"
#include "Path.h"

#include <ctime>
#include <cstdlib>

#define MAX_RAND_LENGTH 10

ScreenProvider::ScreenProvider(SPType type) 
    : _bytespixel(4), _type(type) {
}

/* Start SampleProvider */
bool ScreenProvider::startSample() {
    if(!_samp) return false;
    else
    {
        _samp->startCWT();
        return true;
    }
}

/* is it ready */
bool ScreenProvider::isSampleReady() {
    return _samp->isReady();
}

FrameBuffer * ScreenProvider::getFrameBuffer() {
    return _samp->getFrameBuffer();
}

void ScreenProvider::setBounds(int l, int r, int t, int b) {
    _bounds.set(l, t, r, b);
}

/* sample provider control */
void ScreenProvider::samplePause()    { return _samp->pause(); }
void ScreenProvider::sampleResume()   { return _samp->resume(); }
bool ScreenProvider::isSamplePaused() { return _samp->isPause(); }

/* append the random number */
void ScreenProvider::setWIDAppdx()
{
}

/* ScreenProviderPartial */
ScreenProviderPartial:: ScreenProviderPartial(CapImageRect bounds, unsigned int frequency)
        : ScreenProvider(bounds)
{
    MonitorVectorProvider mvp(true, true);
    if(!mvp.getMonByBounds(_bounds, _monitor)) {
        _bounds.setInvalid();
    }
    else
    _samp = new SamplesProvider(_bounds, _monitor, frequency);

}

void ScreenProviderPartial::init() {
}

/* ScreenProviderWindow for pid */
ScreenProviderWindow::ScreenProviderWindow(Pid pid, unsigned int frequency)
    : ScreenProvider(SP_WINDOW), _pid(pid)
{
    WindowVectorProvider wvp(pid);
    wvp.getWinByPid(pid, _win);
    _samp = new SamplesProvider(_win, frequency);
    setBounds(0, _win.getWidth(), 0, _win.getHeight());
}

/* ScreenProviderWindow for handler */
ScreenProviderWindow::ScreenProviderWindow(size_t hd, unsigned int frequency)
    : ScreenProvider(SP_WINDOW), _hd(hd)
{
    WindowVectorProvider wvp(-1);
    wvp.getWinByHandler(hd, _win);
    _samp = new SamplesProvider(_win, frequency);
    setBounds(0, _win.getWidth(), 0, _win.getHeight());
}

void ScreenProviderWindow::init()
{
}

void ScreenProviderMonitor::init(unsigned int frequency)
{
    MonitorVectorProvider mvp(true, true);
    mvp.getMonByID(_id, _monitor);
    _samp = new SamplesProvider(_monitor, frequency);
    setBounds(0, _monitor.getOrgWidth(), 0, _monitor.getOrgHeight());
}

ScreenProviderMonitor::ScreenProviderMonitor(int id, unsigned int frequency)
    : ScreenProvider(SP_MONITOR)
{
    _id = id;
    init(frequency);
}

// Format: "MONITOR_1594208741_ID478241813_RND1860363629"
