#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

enum SPImageType { SP_IMAGE_BGRA, SP_IMAGE_RGBA, SP_IMAGE_RGB, SP_IMAGE_YUV420};  // default RGBA

class CapImageRect;
struct ImageBGRA {
    unsigned char B, G, R, A;
};

class FrameBuffer {
  public:
    explicit FrameBuffer(size_t size) : _size(size), _capacity(size),
                            _subCapacity(0), _subData(nullptr),
                            _width(0), _height(0)
    {
        _isUsed.store(true, std::memory_order_relaxed);
        if(size == 0)
            _data = nullptr;
        else
            _data = (unsigned char *) malloc (size);
        _isValid = false;
    }
    explicit FrameBuffer(const CapImageRect & bounds, unsigned int bytespp) { }
    FrameBuffer() : FrameBuffer(0) { }
    ~FrameBuffer() { reSet(0); resetSubData(0); }

    size_t getSize() const   { return _size ; }
    size_t getCapacity() const { return _capacity; }
    unsigned char * getData() const { return _data; }
    unsigned char * getDataToWrite() { return _data; }

    void setData(unsigned char * data, size_t w, size_t h, SPImageType type=SPImageType::SP_IMAGE_RGBA);
    void setDataPerRow(unsigned char * data, int w, int h,
                       int bytesrow, SPImageType type=SPImageType::SP_IMAGE_RGBA);

    /*
     * convert
     * received order is B, G, R, A
     * Convert to R, G, B, A
     */
    void ConvertBGRA2RGBA(unsigned char * dst, size_t size);
    void ConvertBGRA2RGBA() { return ConvertBGRA2RGBA(_data, _size); }
    void ConvertBGRA2YCrCb420(unsigned char * dst, size_t size);
    void ConcertBGRA2RGB(unsigned char * dst, unsigned char * src, size_t srcSize);

    bool reSet(size_t size);
    void reSet(const CapImageRect & bounds, unsigned int bytespp);
    void reSet() { reSet(0);}

    void resetSubData(size_t size);
    void resetSubData() { resetSubData(0); }
    unsigned char * getSubData() { return _subData; }
    [[nodiscard]] size_t getSubCap() const { return _subCapacity; }
    [[nodiscard]] bool isUsed() const { return _isUsed.load(std::memory_order_relaxed) ;}

    bool set(unsigned char * data, uint64_t size);
    // comsumer needs to set frame as unsed.
    void setUsed() { _isUsed.store(true, std::memory_order_relaxed); }
    void setUnused() { _isUsed.store(false, std::memory_order_relaxed); }

    void setInvalid() {_isValid = false;}

    void setWidthHeight(size_t w, size_t h);

    [[nodiscard]] size_t getWidth() const { return _width; }
    [[nodiscard]] size_t getHeight() const { return _height; }

  private:
    size_t   _size;
    size_t   _capacity;
    size_t   _subCapacity;
    unsigned char * _data;
    unsigned char * _subData;
    bool     _isValid;
    std::atomic<bool>     _isUsed;
    size_t   _width;
    size_t   _height;
};

/*
 *  Circular buffer for reading and writting
 *  At least 2 framebuffer would be used in the circle to
 *  differentiate the start and end pointer.
 */
template<typename T>
class CircleWRBuf {
  public:
    explicit CircleWRBuf(int);
    ~CircleWRBuf();
    CircleWRBuf() : CircleWRBuf (4) { }

    [[nodiscard]] bool empty() const {
        return read == write || data[read % size].isUsed();
    }

    [[nodiscard]] bool full () const {
        return size == 0 || !data[write % size].isUsed() || (read == ((write+1) % size));
    }

    void setEmpty() {
        std::scoped_lock<std::mutex> guard_w(_mtx);
        read = write;
        for (int i=0; i<size; i++) data[i].setUsed();
    }

    T * getToWrite();
    T * getToRead () ;
    [[nodiscard]] int getSize () const { return size; }

    void clear();

  private:
    int size;
    T *data;

    mutable std::mutex _mtx;
    int write;
    int read ;
};

template<typename T> CircleWRBuf<T>::CircleWRBuf(int sz) : size(sz), write(0), read(0) {
    if (sz==0) throw std::invalid_argument("size cannot be zero");
    data = new T[sz];
}

template<typename T> CircleWRBuf<T>::~CircleWRBuf() {
    delete[] data;
}

/* returns true if write was successful, false if the buffer is already full */
template<typename T> T * CircleWRBuf<T>::getToWrite() {
    std::scoped_lock<std::mutex> guard_w(_mtx);
    if (full()) return nullptr;

    T * ret = &data[write];
    write = (write + 1) % size;
    return ret;
}

/* returns true if there is something to remove, false otherwise */
template<typename T> T * CircleWRBuf<T>::getToRead() {
    std::scoped_lock<std::mutex> guard_w(_mtx);
    if (empty()) return nullptr;

    int ret = read;
    read = (read + 1) % size;
    return &data[ret];
}

template<typename T> void CircleWRBuf<T>::clear() {
    delete[] data;
    std::scoped_lock<std::mutex> guard_w(_mtx);
    size = read = write = 0;
}

#endif //_BUFFER_H_
