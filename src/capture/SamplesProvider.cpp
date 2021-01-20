#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <assert.h>
#include "SamplesProvider.h"

FrameProcessorWrap* FrameProcessorWrap::_instance = 0;

FrameProcessorWrap::FrameProcessorWrap() :
                _duration(std::chrono::microseconds(MICROSECONDS_PER_SECOND/DEFAULT_SAMPLE_PROVIDER)),
                _fpi(0),
                _isPause(true),
                _isReady(false),
                _imgType(SPImageType::SP_IMAGE_RGBA)
{ }

void FrameProcessorWrap::setImageTypeToYUV ()
{
    _imgType = SPImageType::SP_IMAGE_YUV;
}

bool FrameProcessorWrap::isYUVType ()
{
    return _imgType == SPImageType::SP_IMAGE_YUV;
}

FrameProcessorWrap * FrameProcessorWrap::instance ()
{
    if(_instance == 0) {
        _instance = new FrameProcessorWrap();
    }
    return _instance;
}

void FrameProcessorWrap::setCFB(CircWRBuf<FrameBuffer> *fb)
{
    _fb = fb;
}

void FrameProcessorWrap::setMV(CapMonitor * mon, unsigned int frequency)
{
    _type = SP_MONITOR;
    _monitor = mon;
    _duration = std::chrono::microseconds(MICROSECONDS_PER_SECOND/frequency);
}

void FrameProcessorWrap::setBD(CapImageRect * bd)
{
    _type = SP_PARTIAL;
    _bounds = bd;
}

CapMonitor * FrameProcessorWrap::getMonitor()
{
    return _monitor;
}

void FrameProcessorWrap::debug(char * array [])
{
    std::cout << "Hello from FrameProcessorWrap::debug" << std::endl;
    while( (*array) != nullptr) {
            std::cout << "Hello from FrameProcessorWrap::debug available: " << *array << std::endl;
    }
}

bool FrameProcessorWrap::isPartial()
{
    return (_type == SP_PARTIAL);
}

void FrameProcessorWrap::writeBuf(CapMonitor * mon, unsigned char * buf,
                                  int bpr, size_t bufferSize)
{
    FrameBuffer * fb = _fb->getToWrite();
    if(bufferSize) {
        fb->reSet(bufferSize);
    }

    if(fb) {
        fb->setData(buf, mon->getOrgWidth()*mon->getOrgHeight()*4, !isYUVType());
//        std::cout << "queu is write ..." << std::endl;
    } else {
        pause();
//        std::cout << "queu is full, pause..." << std::endl;
    }
}

void FrameProcessorWrap::writeBuf(CapImageRect * bd, unsigned char * buf,
                                  int bpr, size_t bufferSize)
{
    FrameBuffer * fb = _fb->getToWrite();
    if(fb) {
//std::cout << "bd->getWidth(): " << bd->getWidth() << " bd->getHeight(): " <<  bd->getHeight() << std::endl;
//            fb->setData(buf, bd->getWidth()*bd->getHeight()*4);
        if(bufferSize) {
            fb->reSet(bufferSize);
        }

        fb->setDataPerRow(buf, bd->getWidth(), bd->getHeight(), bpr, !isYUVType());
//        std::cout << "write Buf queue is write ..." << std::endl;
    } else {
        pause();
//        std::cout << "write Buf queue is full, pause..." << std::endl;
    }

}

void CircleWriteThread::startFPW()
{
    FrameProcessorWrap::instance()->setCFB(_fb);
    FrameProcessorWrap::instance()->init();
    FrameProcessorWrap::instance()->resume();
}

void CircleWriteThread::mainImp()
{
    assert(_fb);
    FrameBuffer *fb;

    /*
     * start a singleton to receive the capture from kernel's call back
     * For IOS and (Monitor or Partial) capture, start new thread to capture.
     * Otherwise, do while loop to call CircleWriteThread::WindowsFrame to get frame
     */
    if( PLATFORM == SHAREDT_IOS && (_type == SP_MONITOR
        || _type == SP_PARTIAL) ) {
        /* start FrameProcessorWrap */
        startFPW();
    } else {
        _isReady = true;
        FrameBuffer * fb;
        while ( true )
        {
            auto start = std::chrono::system_clock::now();
            fb = _fb->getToWrite();
            if(fb) {
                if(!WindowsFrame(fb)) { fb->setInvalid(); }
            } else {
//                std::cout << "queu is full, pause..." << std::endl;
                std::this_thread::sleep_for(5ms);
            }
            std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
            if(_duration > diff) {
                std::this_thread::sleep_for(_duration - diff);
            }
        }
    }

    return;
}

FrameBuffer * SamplesProvider::getFrameBuffer() {
    FrameBuffer * buf = const_cast<FrameBuffer *>(_buffer.getToRead ());
    if(buf == nullptr) {
//        std::cout << "buf null" << std::endl;
    } else {
        if(isPause())  {
            resume();
        }
//        std::cout << "buf not null" << std::endl;
    }

    return buf;
}

bool CircleWriteThread::isReady() {
    if(PLATFORM == SHAREDT_IOS && (_type == SP_MONITOR || _type == SP_PARTIAL))
        return FrameProcessorWrap::instance()->isReady();
    else
        return _isReady;
}

void CircleWriteThread::pause() {
    if(_type == SP_MONITOR || _type == SP_PARTIAL)
        return FrameProcessorWrap::instance()->pause();
    std::lock_guard<std::mutex> lk(_mtx);
    _isPause = true;
}

bool CircleWriteThread::isPause() {
    if(_type == SP_MONITOR || _type == SP_PARTIAL)
        return FrameProcessorWrap::instance()->isPause();
    std::lock_guard<std::mutex> lk(_mtx);
    return _isPause;

}

void CircleWriteThread::resume() {
    if(_type == SP_MONITOR || _type == SP_PARTIAL)
        return FrameProcessorWrap::instance()->resume();
    std::lock_guard<std::mutex> lk(_mtx);
    _isPause = false;

}

bool SamplesProvider::isReady() {
    return _cwt.isReady();
}
