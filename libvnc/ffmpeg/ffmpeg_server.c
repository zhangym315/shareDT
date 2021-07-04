#include "ffmpeg_server.h"

rfbBool
rfbSendRectEncodingFFMPEG(rfbClientPtr cl,
                          int x,
                          int y,
                          int w,
                          int h)
{
    return rfbSendRectEncodingRaw (cl, x, y, w, h);
}
