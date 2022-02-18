#ifndef __SCREENPROVIDER_H__
#define __SCREENPROVIDER_H__

#include <algorithm>
#include <TypeDef.h>
#include <WindowsProvider.h>
#include "ImageRect.h"
#include "WindowProcessor.h"
#include "Buffer.h"

class SamplesProvider;
using namespace std;
//using namespace Screen_Capture;

typedef std::vector<CapWindow> Windows;

class ScreenProvider {
  public:

    /* Constructor */
    ScreenProvider() : ScreenProvider(SP_ALLMONITOR) {}  /* default is all monitor */

    /* for both SP_ALLMONITOR type and all other types */
    ScreenProvider(SPType type);

    /* For partial type */
    ScreenProvider(CapImageRect bounds) : ScreenProvider(SP_PARTIAL) {
        _bounds = bounds;
    }

    ~ScreenProvider() {  }

    const CapImageRect & getBounds() { return _bounds; }
    virtual void init() { }

    /* default, return an invalid monitor */
    virtual bool isValid() { return true; }
    virtual bool startSample() ;
    virtual bool isSampleReady();
    FrameBuffer * getFrameBuffer ();
    void setBounds(int l, int r, int t, int b);
    int getWidth()  const { return _bounds.getWidth(); }
    int getHeight() const { return _bounds.getHeight(); }
    int getOffsetX() const { return _bounds.getTLX(); }
    int getOffsetY() const { return _bounds.getTLY(); }

    void samplePause()    ;
    void sampleResume()   ;
    bool isSamplePaused() ;

    void setWIDPrefix() ;
    void setWIDAppdx() ;
    const String & getWID();

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
    ScreenProviderWindow(Pid pid, unsigned int frequency);
    void init() override;
    bool isValid() override { return _win.isValid(); }

  private:
    Pid                  _pid;
    size_t               _hd;
    CapWindow            _win;
};

class ScreenProviderMonitor final : public ScreenProvider {
  public:
    ScreenProviderMonitor(int id, unsigned int frequency) ;
    void init(unsigned int frequency);

    bool isValid() override { return _monitor.isValid(); }
    const CapMonitor & get() const { return _monitor; }

  private:
    int           _id;      /* monitor id */
    CapMonitor    _monitor;
};

#endif