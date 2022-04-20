#include "WindowProcessor.h"
#include <atomic>
/*
 * Singleton to provide the capture info
 */
class CaptureInfo {
  public:
    static CaptureInfo * instance() ;

    void setCapMonitor(CapMonitor & cap) { _monPtr = &cap; }
    CapMonitor & getCapMonitor() { return *_monPtr; }

    void setIsRunning(bool isDown);
    bool isRunning() const;
  private:
    CaptureInfo();
    static CaptureInfo * _instance;

    SPType _type;
    CapMonitor * _monPtr{};
    std::atomic<bool> _isServerRunning{};
};
