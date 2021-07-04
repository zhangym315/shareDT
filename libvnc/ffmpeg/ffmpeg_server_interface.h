#ifndef SHAREDT_FFMPEG_SERVER_INTERFACE_H
#define SHAREDT_FFMPEG_SERVER_INTERFACE_H

#include <libavcodec/avcodec.h>
#include <rfb/rfb.h>
#include "ffmpeg_interface.h"

extern AVPacketBuf * encode(AVCodecContext *enc_ctx, AVFrame *frame);

#endif //SHAREDT_FFMPEG_SERVER_INTERFACE_H
