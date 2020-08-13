#include "Buffer.h"
#include "ImageRect.h"
FrameBuffer::FrameBuffer(const CapImageRect & bounds, unsigned int bytespp) :
    FrameBuffer((bounds.getHeight ()) * (bounds.getWidth ()) * bytespp) {  }

void FrameBuffer::reSet(const CapImageRect & bounds, const unsigned int bytespp)
{
    reSet(bounds.getHeight() * bounds.getWidth() * bytespp);
}
