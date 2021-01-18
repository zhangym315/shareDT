#include "Buffer.h"
#include "ImageRect.h"
FrameBuffer::FrameBuffer(const CapImageRect & bounds, unsigned int bytespp) :
    FrameBuffer((bounds.getHeight ()) * (bounds.getWidth ()) * bytespp) {  }

void FrameBuffer::reSet(const CapImageRect & bounds, const unsigned int bytespp)
{
    reSet(bounds.getHeight() * bounds.getWidth() * bytespp);
}

void FrameBuffer::setData(unsigned char * data, int size, bool convert)
{
    reSet(size);
    std::memcpy(_data, data, size);
    if( convert ) {
        ConvertBGRA2RGBA(_data, size);
    }
    _isValid = true;
}

void FrameBuffer::setDataPerRow(unsigned char * data, int w, int h, int bytesrow, bool convert)
{
    reSet(w * h * 4);
    int perRow = w * 4;
    for (int i=0; i<h; i++)
    {
        std::memcpy(_data + perRow * i, data+i*bytesrow, perRow);
        if( convert ) {
            ConvertBGRA2RGBA(_data + perRow * i, perRow);
        }
    }
    _isValid = true;
}

void FrameBuffer::reSet(const size_t size)
{
    /* reset to 0 size */
    if(size == 0)
    {
        if(_data) free(_data);
        _data = NULL;
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
