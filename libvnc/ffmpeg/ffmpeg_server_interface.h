#ifndef SHAREDT_FFMPEG_SERVER_INTERFACE_H
#define SHAREDT_FFMPEG_SERVER_INTERFACE_H

#include <libavcodec/avcodec.h>
#include <rfb/rfb.h>
#include "ffmpeg_interface.h"

extern rfbBool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacketBuf * out_buffer);

typedef struct ffmpeg_server_ctx {
    AVCodecContext * codec_ctx;
    struct SwsContext * sws_ctx;
    AVFrame *  av_frame;
    AVPacketBuf buf;
}ffmpeg_server_ctx_t;

#endif //SHAREDT_FFMPEG_SERVER_INTERFACE_H
