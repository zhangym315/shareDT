#include <cstdio>
#include <cassert>
#include <chrono>
#include <iostream>

#include "SamplesProvider.h"

using namespace std::chrono_literals;

CapMonitor * FrameGetterSystem::getMonitor()
{
    return _monitor;
}

void FrameGetterSystem::debug(char * array [])
{
    while( (*array) != nullptr) {
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
        fb->setData(buf, mon->getOrgWidth(), mon->getOrgHeight());
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

        fb->setDataPerRow(buf, bd->getWidth(), bd->getHeight(), bpr);
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
            if(!FrameGetter::windowsFrame(fb, _type, id)) {
                fb->setInvalid();
            }
        } else {
            std::this_thread::sleep_for(5ms);
        }
        std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
        if(_duration > diff) {
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
