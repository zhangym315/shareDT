#include "Buffer.h"
#include "ImageRect.h"
#include "SamplesProvider.h"
#include "RGB2YUV.h"

FrameBuffer::FrameBuffer(const CapImageRect & bounds, unsigned int bytespp) :
    FrameBuffer((bounds.getHeight ()) * (bounds.getWidth ()) * bytespp) {  }

void FrameBuffer::reSet(const CapImageRect & bounds, const unsigned int bytespp)
{
    reSet(bounds.getHeight() * bounds.getWidth() * bytespp);
}

void FrameBuffer::setData(unsigned char * data, int size, bool isYuv)
{
    reSet(size);
    std::memcpy(_data, data, size);
    if( isYuv ) {
        ConvertBGRA2YCrCb420(_data, size);
    } else
    {
        ConvertBGRA2RGBA(_data, size);
    }
    _isValid = true;
}

void FrameBuffer::setDataPerRow(unsigned char * data, int w, int h, int bytesrow, bool isYuv)
{
    size_t total = w * h * 4;
    reSet(total);

    int perRow = w * 4;
    for (int i=0; i<h; i++)
    {
        std::memcpy(_data + perRow * i, data+i*bytesrow, perRow);
        if ( !isYuv )
            ConvertBGRA2RGBA(_data + perRow * i, perRow);
    }

    if( isYuv ) {
        ConvertBGRA2YCrCb420(_data, total);
    }

    _isValid = true;
}

void FrameBuffer::reSet(const size_t size)
{
    /* reset to 0 size */
    if(size == 0)
    {
        if(_data) free(_data);
        _data = nullptr;
        _size = _capacity = 0;
        return ;
    }

    _size = size;

    /*
     * Only if requested size is greater,
     * If size < _capacity, there is no need to re-allocate
     */
    if(_capacity < size)
    {
        if(_data) free(_data);
        _data = (unsigned char * ) malloc (size) ;
        _capacity = size;
    }
}

void FrameBuffer::resetSubData(const size_t size)
{
    if (size == 0 )
    {
       if(_subData) free(_subData);
       _subData = nullptr;
       _subCapacity = 0;
       return;
    }

    if (_subCapacity < size)
    {
        if(_subData) free(_subData);
        _subData = (unsigned char * ) malloc (size) ;
        _subCapacity = size;
    }
}

void FrameBuffer::ConvertBGRA2RGBA(unsigned char * dst, size_t size)
{
    struct ImageBGRA * data = (struct ImageBGRA * ) dst;
    unsigned char tmp;
    int rowStart;
    for (int i=0; i < size/4; i++)
    {
        rowStart = i * 4;
        tmp = data[i].B;
        *(dst + rowStart + 0) = data[i].R;
//            *(dst + rowStart + 1) = data[i].G;  G is no need to shift
        *(dst + rowStart + 2) = tmp;
        *(dst + rowStart + 3) = 255;  /* alpha channel */
    }
}

void FrameBuffer::ConvertBGRA2YCrCb420(unsigned char * dst, size_t size)
{
    struct ImageBGRA * data = (struct ImageBGRA * ) dst;
    unsigned char tmp;
    int rowStart;

    resetSubData(size/4);
    unsigned char * Y  = dst;
    unsigned char * Cb = getSubData();
    unsigned char * Cr = getSubData() + getSubCap()/2;
    unsigned char * half = Cr;
    unsigned char * end = getSubData() + getSubCap();

    for (int i=0; i<size/4 && Cb<half && Cr<end ; i++)
    {
        *Y = CRGB2Y(data[i].R, data[i].G, data[i].B); Y++;
        if ((i & 1) == 0) {
            *Cb = CRGB2Cb(data[i].R, data[i].G, data[i].B); Cb++;
        } else {
            *Cr = CRGB2Cr(data[i].R, data[i].G, data[i].B); Cr++;
        }
    }
}
