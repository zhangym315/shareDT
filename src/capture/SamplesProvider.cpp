#include <cstdio>
#include <cassert>
#include <chrono>
#include <iostream>

#include "SamplesProvider.h"

extern "C" {
#include "TimeUtil.h"
}

using namespace std::chrono_literals;

void FrameGetterSystem::setImageTypeToRGB ()
{
    _imgType = SPImageType::SP_IMAGE_RGB;
}

CapMonitor * FrameGetterSystem::getMonitor()
{
    return _monitor;
}

void FrameGetterSystem::debug(char * array [])
{
    std::cout << "Hello from FrameGetterSystem::debug" << std::endl;
    while( (*array) != nullptr) {
            std::cout << "Hello from FrameGetterSystem::debug available: " << *array << std::endl;
    }
}

bool FrameGetterSystem::isPartial()
{
    return (_type == SP_PARTIAL);
}

void FrameGetterSystem::writeBuf(CapMonitor * mon, unsigned char * buf,
                                  int bpr, size_t bufferSize)
{
    FrameBuffer * fb = _fb->getToWrite();

    if(fb) {
        if(bufferSize) {
            fb->reSet(bufferSize);
        }
        fb->setData(buf, mon->getOrgWidth(), mon->getOrgHeight(), _imgType);
    } else {
        pause();
    }
}

void FrameGetterSystem::writeBuf(CapImageRect * bd, unsigned char * buf,
                                  int bpr, size_t bufferSize)
{
    FrameBuffer * fb = _fb->getToWrite();
    if(fb) {
        if(bufferSize) {
            fb->reSet(bufferSize);
        }

        fb->setDataPerRow(buf, bd->getWidth(), bd->getHeight(), bpr, getImageType());
    } else {
        pause();
    }

}

void FrameGetterThread::mainImp()
{
    assert(_fb);

    FrameBuffer * fb;
    int id = _type == SP_WINDOW ? _win->getHandler() : _mon->getId();

    _isReady.store(true, std::memory_order_relaxed);
    while (!isStopped())
    {
        auto start = std::chrono::system_clock::now();
        fb = _fb->getToWrite();
        if(fb) {
            if(!FrameGetter::windowsFrame(fb, _type, id, SPImageType::SP_IMAGE_RGBA)) { fb->setInvalid(); }
        } else {
            std::this_thread::sleep_for(5ms);
        }
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
        if(_duration > diff) {
            std::cout << get_current_time_string() << " FrameGetterThread sleeped " << _duration.count() << std::endl;
            std::this_thread::sleep_for(_duration - diff);
        }
    }
}

FrameBuffer * SamplesProvider::getFrameBuffer() {


    auto * buf = const_cast<FrameBuffer *>(_buffer.getToRead ());
    auto diff = std::chrono::system_clock::now() - _start;

    if(_duration > diff) {
        std::this_thread::sleep_for(_duration - diff);
    }
    _start = std::chrono::system_clock::now();

    if(isPause()) resume();
    return buf;
}
