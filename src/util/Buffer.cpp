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

void FrameBuffer::setData(unsigned char * data, int size, SPImageType type)
{
    reSet(size);
    std::memcpy(_data, data, size);
    unsigned char * tmp1 = _data;
    unsigned char * tmp = data;
    int i = 0;

    switch (type) {
    case SPImageType::SP_IMAGE_YUV420:
        ConvertBGRA2YCrCb420(_data, size);
        break;

    case SPImageType::SP_IMAGE_RGBA:
        ConvertBGRA2RGBA(_data, size);
        break;

    case SPImageType::SP_IMAGE_RGB:
        while (i<size) {
            i++;
            *tmp1 = *tmp;
            if (i % 4 == 0)
            {
                tmp++;
            }
            else
            {
                tmp++;
                tmp1++;
            }
        }
        break;

    default:
        break;

    }

    _isValid = true;
}

void FrameBuffer::setDataPerRow(unsigned char * data, int w, int h, int bytesrow, SPImageType type)
{
    size_t total, perRow;
    unsigned char * tmp1 = _data;
    unsigned char * tmp = data;
    int i = 0;

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
        reSet(total);
        ConvertBGRA2YCrCb420(_data, total);
        break;

    case SPImageType::SP_IMAGE_RGB:
        total = w * h * 3;
        reSet(total);
        while (i<total && i<h*bytesrow) {
            i++;
            *tmp1 = *tmp;
            if (i % 4 == 0)
            {
                tmp++;
            }
            else
            {
                tmp++;
                tmp1++;
            }
        }
        break;

    default:
        break;
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
