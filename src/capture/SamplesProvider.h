#ifndef _SAMPLESPROVIDER_H_
#define _SAMPLESPROVIDER_H_

#include <mutex>
#include "ScreenProvider.h"
#include "WindowProcessor.h"
#include "Thread.h"
#include "Buffer.h"

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

enum THREAD_STATUS { STOP, START, PAUSE, CONTI, NONE };

class FrameProcessorImpl;

/*
 * FrameProcessorWrap is singleton
 * This will be used when system capture the screenshot
 *     and get called back by _fpi
 */
class FrameProcessorWrap {
  public:
    static FrameProcessorWrap * instance() ;
    void init();
    void setCFB(CircWRBuf<FrameBuffer> * fb) ;
    void pause();
    void resume();
    bool isPause() { return _isPause; }
    void setMinFrameDuration(const std::chrono::microseconds & duration);
    void convert();
    void setMV(CapMonitor * mon);
    void setBD(CapImageRect * bd);
    CapMonitor * getMonitor ();
    bool isPartial();
    void writeBuf(CapMonitor * mon, unsigned char * buf, int bpr) ; /* write buffer */
    void writeBuf(CapImageRect * bd, unsigned char * buf, int bpr);
    bool isReady() { return _isReady; }
    CapImageRect * getBounds() { return _bounds; }

  private:
    FrameProcessorWrap();
    static FrameProcessorWrap * _instance;
    CircWRBuf<FrameBuffer>     * _fb;
    class  FrameProcessorImpl * _fpi;
    CapMonitor                * _monitor;
    CapImageRect              * _bounds;
    std::chrono::microseconds   _duration;
    bool                        _isPause;
    bool                        _isReady;
    std::mutex                  _mtx;
    SPType                      _type;
 };

/*
 * Write thread to write to the circle buffer
 * Will have a new thread to capture screenshot
 */
class CircleWriteThread : public Thread {
  public:
    /* this is final constructor */
    CircleWriteThread(CircWRBuf<FrameBuffer> * fb,
                      std::chrono::milliseconds sp) : _fb(fb),
                      _status(NONE), _sleeping(sp), Thread(false),
                      _isReady(false), _isPause(true) {}

    CircleWriteThread(CircWRBuf<FrameBuffer> *fb) :
        CircleWriteThread(fb, std::chrono::milliseconds(1000)) { }
    CircleWriteThread() : CircleWriteThread(nullptr,
                      std::chrono::milliseconds(1000)) { }

    ~CircleWriteThread () {  }

    /* monitor capture thread */
    CircleWriteThread(CircWRBuf<FrameBuffer> * fb,
                      CapMonitor & mon) : CircleWriteThread(fb){
        _mon = &mon;
        _type = SP_MONITOR;
        init() ;
    }

    /* partial capture thread */
    CircleWriteThread(CircWRBuf<FrameBuffer> * fb,
                      CapImageRect & rect) : CircleWriteThread(fb){
        _type = SP_PARTIAL;
        init() ;
    }

    CircleWriteThread(CircWRBuf<FrameBuffer> * fb,
                      CapImageRect & rect, CapMonitor & mon) :
                        CircleWriteThread(fb) {
        _type = SP_PARTIAL;
        _bounds = &rect;
        _mon  = &mon;
        init();
    }

    /* window capture thread */
    CircleWriteThread(CircWRBuf<FrameBuffer> * fb,
                      CapWindow & win) : CircleWriteThread(fb){
        _type = SP_WINDOW;
        _win  = &win;
        init() ;
    }

    void init() ;

    void mainImp();
    void startFPW() ; /* start frame processor */

    /* platform spedific */
    bool WindowsFrame(FrameBuffer * fb);

    bool isReady() ;

    void pause();
    bool isPause();
    void resume();

  private:
    THREAD_STATUS           _status;
    CircWRBuf<FrameBuffer> * _fb;
    std::chrono::milliseconds _sleeping;
    SPType                _type;
    CapWindow           * _win;
    CapMonitor          * _mon;
    CapImageRect        * _bounds;
    bool                  _isPause;
    bool                  _isReady;
    std::mutex            _mtx;
};

/* Provider monitor samples screen */
class SamplesProvider  {
  public:
    SamplesProvider(int size, CapMonitor & mon) :
        _buffer(size), _cwt(&_buffer, mon) {
        if(PLATFORM == SHAREDT_IOS)
            FrameProcessorWrap::instance()->setMV(&mon);
        else {
        }
    }

    SamplesProvider(int size, CapWindow & win) : _buffer(size), _cwt(&_buffer, win) {
    }

    SamplesProvider(int size, CapImageRect & rect, CapMonitor & mon) :
        _buffer(size), _cwt(&_buffer, rect, mon) {
        if(PLATFORM == SHAREDT_IOS) {
            FrameProcessorWrap::instance()->setMV(&mon);
            FrameProcessorWrap::instance()->setBD(&rect);
        } else {

        }
    }

    /* default 10, should be configured */
    SamplesProvider(CapMonitor & mon) : SamplesProvider(20, mon) { }
    SamplesProvider(CapWindow  & win) : SamplesProvider(20, win) {
    }
    SamplesProvider(CapImageRect & bd, CapMonitor & mon) : SamplesProvider(20, bd, mon) { }

    ~SamplesProvider () {
        _buffer.clear();
    }

    void startCWT() { _cwt.go(); }
    CircleWriteThread & getCWT() { return _cwt; }

    FrameBuffer * getFrameBuffer();

    bool isReady() ;

    void pause()   { _cwt.pause(); }
    bool isPause() { return _cwt.isPause(); }
    void resume()  { _cwt.resume(); }

  private:
    CircWRBuf<FrameBuffer> _buffer;
    CircleWriteThread     _cwt;
};

#endif //_SAMPLESPROVIDER_H_