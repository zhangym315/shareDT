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

void FrameBuffer::setData(unsigned char * data, size_t w, size_t h, SPImageType type)
{
    size_t total;

    _width = w;
    _height = h;
    switch (type) {
    case SPImageType::SP_IMAGE_YUV420:
        total = w * h * 4;
        reSet(total);
        std::memcpy(_data, data, total);
        ConvertBGRA2YCrCb420(_data, total);
        break;

    case SPImageType::SP_IMAGE_RGBA:
        total = w * h * 4;
        reSet(total);
        std::memcpy(_data, data, total);
        ConvertBGRA2RGBA(_data, total);
        break;

    case SPImageType::SP_IMAGE_RGB:
        total = w * h * 3;
        reSet(total);
        ConcertBGRA2RGB(_data, data, w * h * 4);
        break;

    default:
        break;

    }

    _isValid = true;
}

void FrameBuffer::setDataPerRow(unsigned char * data, int w, int h, int bytesrow, SPImageType type)
{
    size_t total, perRow, dstPerRow;

    setWidthHeight(w, h);

    switch (type) {
    case SPImageType::SP_IMAGE_RGBA:
        total = w * h * 4;
        perRow = w * 4;
        reSet(total);

        for (int i=0; i<h; i++)
        {
            std::memcpy(_data + perRow * i, data+i*bytesrow, perRow);
            ConvertBGRA2RGBA(_data + perRow * i, perRow);
        }
        break;

    case SPImageType::SP_IMAGE_YUV420:
        total = w * h * 4;
        reSet(total);  // problem ~~ TODO, there is no data copy, _data is not set!!!
        ConvertBGRA2YCrCb420(_data, total);
        break;

    case SPImageType::SP_IMAGE_RGB:
        reSet(w * h * 3);
        for (int i=0; i<h; i++)
        {
            ConcertBGRA2RGB(_data + i*3*w, data + i*bytesrow, bytesrow);
        }

        break;

    default:
        break;
    }

    _isValid = true;
}

bool FrameBuffer::reSet(const size_t size)
{
    /* reset to 0 size */
    if(size == 0)
    {
        if(_data) free(_data);
        _data = nullptr;
        _size = _capacity = 0;
        return true;
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
        if (_data == nullptr)
            return false;
        _capacity = size;
    }

    return true;
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

void FrameBuffer::ConcertBGRA2RGB(unsigned char * dst, unsigned char * src, size_t srcSize)
{
    struct ImageBGRA * data = (struct ImageBGRA * ) src;
    unsigned char tmp;
    int rowStart;
    for (int i=0; i < srcSize/4; i++)
    {
        rowStart = i * 3;
        tmp = data[i].B;
        *(dst + rowStart + 0) = data[i].R;
        *(dst + rowStart + 1) = data[i].G;//  G is no need to shift
        *(dst + rowStart + 2) = tmp;
    }
}

void FrameBuffer::ConvertBGRA2YCrCb420(unsigned char * dst, size_t size)
{
    struct ImageBGRA * data = (struct ImageBGRA * ) dst;
    unsigned char tmp;

    resetSubData(size/4);
    unsigned char * Y  = dst;
    unsigned char * Cb = getSubData();
    unsigned char * Cr = getSubData() + getSubCap()/2;
    unsigned char * half = Cr;
    unsigned char * end = getSubData() + getSubCap();

    for (int i=0; i<size/4 && Cb<half && Cr<end ; i++)
    {
//printf("data[%d]: %.2X %.2X %.2X ", i, data[i].R, data[i].G, data[i].B);
        *Y = CRGB2Y(data[i].R, data[i].G, data[i].B); Y++;
        if ((i & 1) == 0) {
            *Cb = CRGB2Cb(data[i].R, data[i].G, data[i].B); Cb++;
        } else {
            *Cr = CRGB2Cr(data[i].R, data[i].G, data[i].B); Cr++;
        }
    }
}


bool FrameBuffer::set (unsigned char *data, uint64_t size)
{
    if (!reSet (size))
        return false;

    memcpy(_data, data, size);
    _size = size;
    _isUsed = true;
    return true;
}

void FrameBuffer::setWidthHeight(size_t w, size_t h)
{
    _width = w;
    _height = h;
}
