#ifndef __SCREENPROVIDER_H__
#define __SCREENPROVIDER_H__

#include <algorithm>

#include "TypeDef.h"
#include "ImageRect.h"
#include "Buffer.h"
#include "WindowsProvider.h"
#include "WindowProcessor.h"
#include "SamplesProvider.h"

using namespace std;

typedef vector<CapWindow> Windows;

/*
 * @note
 * ScreenProvider is a class to provide captured frame, it will call specific capture type(monitor, window or boundary).
 *
 * Following is call stack for different type of capture:
 *                                          /==> FrameGetterSystem -> store frame to SamplesProvider::_buffer
 * ScreenProvider ==> SamplesProvider  ==> |
 *                                          \==> FrameGetterThread -> store frame to SamplesProvider::_buffer
 *
 */
class ScreenProvider {
  public:

    /* Constructor */
    ScreenProvider() : ScreenProvider(SP_ALLMONITOR) {}  /* default is all monitor */

    /* for both SP_ALLMONITOR type and all other types */
    explicit ScreenProvider(SPType type);

    /* For partial type */
    explicit ScreenProvider(const CapImageRect& bounds) : ScreenProvider(SP_PARTIAL) {
        _bounds = bounds;
    }

    ~ScreenProvider() { delete _samp; }

    const CapImageRect & getBounds() { return _bounds; }
    virtual void init() { }

    /* default, return an invalid monitor */
    virtual bool isValid() { return true; }
    virtual bool startSample() ;
    virtual bool isSampleReady();
    FrameBuffer * getFrameBuffer ();
    void setBounds(int l, int r, int t, int b);
    [[nodiscard]] int getWidth()  const { return _bounds.getWidth(); }
    [[nodiscard]] int getHeight() const { return _bounds.getHeight(); }

    void samplePause()    ;
    void sampleResume()   ;
    bool isSamplePaused() ;

    void setTargetImageType(SPImageType type) { _samp->setTargetImageType(type); }
  protected:
    unsigned int       _bytespixel;
    CapImageRect       _bounds;
    SPType             _type;
    SamplesProvider *  _samp;
};

class ScreenProviderPartial final : public ScreenProvider {
  public:
    ScreenProviderPartial(CapImageRect bounds, unsigned int frequency);
    void init() override;
    bool isValid() override { return _bounds.isValid(); }

  private:
    int           _id;      /* which monitor id, the bounds belongs to */
    CapMonitor    _monitor;
};

class ScreenProviderWindow final : public ScreenProvider {
  public:
    ScreenProviderWindow(size_t hd, unsigned int frequency);
    ScreenProviderWindow(pid_t pid, unsigned int frequency);
    void init() override;
    bool isValid() override { return _win.isValid(); }
    const CapWindow & get() const { return _win; }

  private:
    pid_t                _pid;
    size_t               _hd;
    CapWindow            _win;
};

class ScreenProviderMonitor final : public ScreenProvider {
  public:
    explicit ScreenProviderMonitor(unsigned int frequency);
    ScreenProviderMonitor(int id, unsigned int frequency) ;
    void init(unsigned int frequency);

    bool isValid() override { return _monitor.isValid(); }
    [[nodiscard]] const CapMonitor & get() const { return _monitor; }

  private:
    int           _id;      /* monitor id */
    CapMonitor    _monitor;
};

#endif