#include "WindowProcessor.h"

/*
 * Singleton to provide the capture info
 */
class CaptureInfo {
  public:
    static CaptureInfo * instance() ;

    void setCapMonitor(CapMonitor & cap) { _monPtr = &cap; }
    CapMonitor & getCapMonitor() { return *_monPtr; }

  private:
    CaptureInfo();
    static CaptureInfo * _instance;

    SPType _type;
    CapMonitor * _monPtr;
};
