#ifndef SHAREDT_FFMPEG_SERVER_INTERFACE_H
#define SHAREDT_FFMPEG_SERVER_INTERFACE_H

#include <libavcodec/avcodec.h>
#include <rfb/rfb.h>
#include "ffmpeg_interface.h"

extern AVPacketBuf * encode(AVCodecContext *enc_ctx, AVFrame *frame);

typedef struct ffmpeg_server_ctx {
    AVCodecContext * codec_ctx;
    struct SwsContext * sws_ctx;
    AVFrame *  av_frame;
}ffmpeg_server_ctx_t;

#endif //SHAREDT_FFMPEG_SERVER_INTERFACE_H
