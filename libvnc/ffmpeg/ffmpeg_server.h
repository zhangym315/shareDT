#ifndef SHAREDT_FFMPEG_SERVER_H
#define SHAREDT_FFMPEG_SERVER_H
#include <rfb/rfb.h>


extern rfbBool
rfbSendRectEncodingFFMPEG(rfbClientPtr cl,
                          int x,
                          int y,
                          int w,
                          int h);

#endif //SHAREDT_FFMPEG_SERVER_H
