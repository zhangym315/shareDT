#ifndef _SAMPLESPROVIDER_H_
#define _SAMPLESPROVIDER_H_

#include <algorithm>
#include <mutex>
#include <atomic>
#include <chrono>

#include "WindowProcessor.h"
#include "Thread.h"
#include "Buffer.h"
#include "ScopedPrt.h"

/* define platform specific */
enum PLATFORM_STATUS { SHAREDT_WIN, SHAREDT_IOS, SHAREDT_LINUX, SHAREDT_UNKNOWN };

#ifdef __SHAREDT_WIN__
const static PLATFORM_STATUS PLATFORM = SHAREDT_WIN;
#elif __SHAREDT_IOS__
const static PLATFORM_STATUS PLATFORM = SHAREDT_IOS;
#elif __SHAREDT_LINUX__
const static PLATFORM_STATUS PLATFORM = SHAREDT_LINUX;
#else
const static PLATFORM_STATUS PLATFORM = SHAREDT_UNKNOWN;
#endif

#define DEFAULT_SAMPLE_PROVIDER 50
#define MICROSECONDS_PER_SECOND 1000

class FrameProcessorImpl;

/* platform spedific */
class FrameGetter {
public:
    static bool windowsFrame(FrameBuffer * fb, SPType type, size_t handler);
    static bool exportAllFrameGetter(FrameBuffer * fb, SPType type, size_t handler);
};

class FrameGetterControl {
public:
    FrameGetterControl() : _isPause(true), _isReady(false), _isStopped(false) { }
    ~FrameGetterControl() = default;

    virtual void start()     { resume(); _isStopped.store(false, std::memory_order_relaxed); }
    virtual void pause()     { _isPause.store(true, std::memory_order_relaxed); }
    virtual void resume()    { _isPause.store(false, std::memory_order_relaxed); }
    virtual void stop()      { _isStopped.store(true, std::memory_order_relaxed); }
    virtual bool isPause()   { return _isPause.load(std::memory_order_relaxed); }
    virtual bool isReady()   { return _isReady.load(std::memory_order_relaxed); }
    virtual bool isStopped() { return _isStopped.load(std::memory_order_relaxed); }

protected:
    std::atomic<bool>  _isPause;
    std::atomic<bool>  _isReady;
    std::atomic<bool>  _isStopped;
};

/*
 * @class FrameGetterSystem
 *
 * @note FrameGetterSystem is singleton.
 * This will be used when system capture the screenshot.
 *
 * Currently, following capture supports system capture:
 *      MacOS: monitor capture
 *             bound capture
 *      Windows: window capture
 *
 * This will be internally filling up the _fb(CircleWRBuf).
 *
 */
class FrameGetterSystem : public FrameGetterControl {
  public:
    FrameGetterSystem(CircleWRBuf<FrameBuffer> * fb,
                      std::chrono::milliseconds sp) :
                      _fb(fb), _duration(sp), _fpi(nullptr),
                      _monitor(nullptr), _bounds(nullptr), _win(nullptr),
                      _isReInited(false), _type(SPType::SP_NULL)  { }

    FrameGetterSystem(CircleWRBuf<FrameBuffer> * fb,
                      CapMonitor * mon,
                      unsigned int frequency) :
        FrameGetterSystem(fb, std::chrono::milliseconds(MICROSECONDS_PER_SECOND/frequency)) {
        _type = SP_MONITOR;
        _monitor = mon;
        init();
    }

    FrameGetterSystem(CircleWRBuf<FrameBuffer> * fb,
                      CapImageRect * bd,
                      unsigned int frequency) :
        FrameGetterSystem(fb, std::chrono::milliseconds(MICROSECONDS_PER_SECOND/frequency)) {
        _type = SP_PARTIAL;
        _bounds = bd;
        init();

    }

    FrameGetterSystem(CircleWRBuf<FrameBuffer> * fb,
                      CapWindow * win,
                      unsigned int frequency) :
        FrameGetterSystem(fb, std::chrono::milliseconds(MICROSECONDS_PER_SECOND/frequency)) {
        _type = SP_WINDOW;
        _win = win;
        init();
    }

    CapMonitor * getMonitor ();
    bool isPartial();
    void writeBuf(CapMonitor * mon, unsigned char * buf, int bpr, size_t bufSize=0) ; /* write buffer */
    void writeBuf(CapImageRect * bd, unsigned char * buf, int bpr, size_t bufSize=0);
    void setReInitiated() { _isReInited = true; }
    CapImageRect * getBounds() { return _bounds; }

    void debug(char * array []);

    void pause()  override;
    void resume() override;
    void stop()   override;

private:
    void init();

    CircleWRBuf<FrameBuffer>    * _fb;
    std::chrono::microseconds   _duration;
    FrameProcessorImpl        * _fpi;

    CapMonitor                * _monitor;
    CapImageRect              * _bounds;
    CapWindow                 * _win;
    bool                        _isReInited;  // used by export all monitors
    SPType                      _type;
 };

/*
 * @class FrameGetterThread
 *
 * @note FrameGetterThread is a thread to capture screenshot and fill up _fb(CircleWRBuf).
 *
 * Currently, following capture supports thread capture:
 *      MacOS: window capture
 *      Linux: monitor capture
 *             window capture
 *             bound capture
 *      Windows: monitor capture
 *               bound capture
 *
 */
class FrameGetterThread : public Thread, public FrameGetterControl {
  public:
    /* this is final constructor */
    FrameGetterThread(CircleWRBuf<FrameBuffer> * fb,
                      std::chrono::milliseconds sp) : _fb(fb),
                      _duration(sp), _type(SPType::SP_NULL), _win(nullptr),
                      _mon(nullptr), _bounds(nullptr), Thread(false),
                      FrameGetterControl() { }

    FrameGetterThread(CircleWRBuf<FrameBuffer> *fb, unsigned int frequency) :
        FrameGetterThread(fb, std::chrono::milliseconds(MICROSECONDS_PER_SECOND/frequency)) { }
    FrameGetterThread() : FrameGetterThread(nullptr,
                      std::chrono::milliseconds(MICROSECONDS_PER_SECOND)) { }

    ~FrameGetterThread () = default;

    /* monitor capture thread */
    FrameGetterThread(CircleWRBuf<FrameBuffer> * fb,
                      CapMonitor & mon,
                      unsigned int frequency) : FrameGetterThread(fb, frequency)
    {
        _mon = &mon;
        _type = SP_MONITOR;
        init() ;
    }

    /* partial capture thread */
    [[maybe_unused]] FrameGetterThread(CircleWRBuf<FrameBuffer> * fb,
                      CapImageRect & rect,
                      unsigned int frequency) : FrameGetterThread(fb, frequency)
    {
        _type = SP_PARTIAL;
        init() ;
    }

    FrameGetterThread(CircleWRBuf<FrameBuffer> * fb,
                      CapImageRect & rect,
                      CapMonitor & mon,
                      unsigned int frequency) : FrameGetterThread(fb, frequency)
    {
        _type = SP_PARTIAL;
        _bounds = &rect;
        _mon  = &mon;
        init();
    }

    /* window capture thread */
    FrameGetterThread(CircleWRBuf<FrameBuffer> * fb,
                      CapWindow * win,
                      unsigned int frequency) : FrameGetterThread(fb, frequency)
    {
        _type = SP_WINDOW;
        _win  = win;
        init() ;
    }

    void start() override { go(); FrameGetterControl::start(); }

private:
    void init();
    void mainImp() override;

    CircleWRBuf<FrameBuffer> * _fb;
    std::chrono::milliseconds _duration;
    SPType                _type;
    CapWindow           * _win;
    CapMonitor          * _mon;
    CapImageRect        * _bounds;
//    std::mutex            _mtx;
};

class SamplesProvider  {
  public:
    SamplesProvider(unsigned int frequency) : SamplesProvider( frequency, SPImageType::SP_IMAGE_RGBA) { }
    SamplesProvider(unsigned int frequency, SPImageType type) : _buffer(5, type),
                        _start(std::chrono::system_clock::now()),
                        _duration(std::chrono::milliseconds(MICROSECONDS_PER_SECOND/frequency)) { }

    SamplesProvider(CapMonitor & mon, unsigned int frequency) : SamplesProvider(frequency)
    {
#ifdef __SHAREDT_IOS__
        _fgc = std::make_unique<FrameGetterSystem>(&_buffer, &mon, frequency);
#else
        _fgc = std::make_unique<FrameGetterThread>(&_buffer, mon, frequency);
#endif

    }

    SamplesProvider(CapWindow  & win, unsigned int frequency) : SamplesProvider(frequency)
     {
         _fgc = std::make_unique<FrameGetterThread>(&_buffer, &win, frequency);
     }

    SamplesProvider(CapImageRect & bd, CapMonitor & mon, unsigned int frequency) : SamplesProvider(frequency)
    {
#ifdef __SHAREDT_IOS__
        _fgc = std::make_unique<FrameGetterSystem>(&_buffer, &bd, frequency);
#else
        _fgc = std::make_unique<FrameGetterThread>(&_buffer, bd, mon, frequency);
#endif

    }

    ~SamplesProvider () {
        _buffer.clear();
    }

    /* read frame buffer */
    FrameBuffer * getFrameBuffer();
    void setTargetImageType(SPImageType type) { _buffer.setImageType(type); }

    /* capture control */
    void start()    { _fgc->start(); }
    void pause()    { _fgc->pause(); }
    void resume()   { _fgc->resume(); }
    bool isPause()  { return _fgc->isPause(); }
    bool isReady()  { return _fgc->isReady(); }
    std::chrono::microseconds getDuration() { return _duration; }

  private:
    CircleWRBuf<FrameBuffer> _buffer;
    std::unique_ptr<FrameGetterControl> _fgc;
    std::chrono::time_point<std::chrono::system_clock> _start;
    std::chrono::microseconds   _duration;
};

#endif //_SAMPLESPROVIDER_H_
