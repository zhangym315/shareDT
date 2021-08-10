#ifndef SHAREDT_FFMPEG_CLIENT_H
#define SHAREDT_FFMPEG_CLIENT_H
#include <rfb/rfb.h>


extern rfbBool
rfbReceiveRectEncodingFFMPEG(rfbClient* cl,
                             rfbFramebufferUpdateRectHeader * rect);

#endif //SHAREDT_FFMPEG_CLIENT_H
