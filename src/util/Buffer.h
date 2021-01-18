#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <memory>

class CapImageRect;
struct ImageBGRA {
    unsigned char B, G, R, A;
};

class FrameBuffer {
  public:
    FrameBuffer(size_t size) : _size(size), _capacity(size)
    {
        if(size == 0)
            _data = NULL;
        else
            _data = (unsigned char *) malloc (size);
        _isValid = false;
    }
    FrameBuffer(const CapImageRect & bounds, unsigned int bytespp) ;
    FrameBuffer() : FrameBuffer(0) { }

    [[nodiscard]] size_t getSize() const   { return _size ; }
    [[nodiscard]] size_t getPacity() const { return _capacity; }
    unsigned char * getData() { return _data; }

    void setData(unsigned char * data, int size, bool convert=true);
    void setDataPerRow(unsigned char * data, int w, int h,
                       int bytesrow, bool convert=true);

    /*
     * convert
     * received order is B, G, R, A
     * Convert to R, G, B, A
     */
    void ConvertBGRA2RGBA(unsigned char * dst, size_t size);
    void ConvertBGRA2RGBA() { return ConvertBGRA2RGBA(_data, _size); }

    void reSet(size_t size);
    void reSet(const CapImageRect & bounds, unsigned int bytespp);
    void reSet() { reSet(0);}

    void setInvalid() {_isValid = false;}

  private:
    size_t   _size;
    size_t   _capacity;
    unsigned char * _data;
    bool     _isValid;
};

/*
 *  Circular buffer for reading and writting
 */
template<typename T>
class CircWRBuf {
  public:
    CircWRBuf(int);
    ~CircWRBuf();
    CircWRBuf() : CircWRBuf (10) { }

    bool empty() const { return read == write; }
    bool full () const { return read == ((write+1) % size); }

    T * getToWrite();
    const T * getToRead () ;
    const int getSize () const { return size; }

    void clear();

  private:
    const int size;
    T *data;
    int write;
    int read ;
};

template<typename T> CircWRBuf<T>::CircWRBuf(int sz): size(sz) {
    if (sz==0) throw std::invalid_argument("size cannot be zero");
    data = new T[sz];
    write = 0;
    read = 0;
}

template<typename T> CircWRBuf<T>::~CircWRBuf() {
    delete data;
}

/* returns true if write was successful, false if the buffer is already full */
template<typename T> T * CircWRBuf<T>::getToWrite() {
    if ( full() ) {
        return nullptr;
    } else {
        T * ret = &data[write];
        write = (write + 1) % size;
        return ret;
    }
}

/* returns true if there is something to remove, false otherwise */
template<typename T> const T * CircWRBuf<T>::getToRead() {
    if ( empty() ) {
        return nullptr;
    } else {
        int ret = read;
        read = (read + 1) % size;
        return &data[ret];
    }
}

template<typename T> void CircWRBuf<T>::clear() {
    delete[] data;
}

#endif //_BUFFER_H_