#ifndef SHAREDT_FFMPEG_CLIENT_INTERFACE_H
#define SHAREDT_FFMPEG_CLIENT_INTERFACE_H
#include <libavcodec/avcodec.h>
#include <rfb/rfb.h>

rfbBool fetch_frame(AVCodecContext *dec_ctx, AVFrame *frame);
rfbBool decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);

typedef struct ffmpeg_client_ctx {
    const AVCodec * codec;
    AVCodecContext * codec_ctx;
    struct SwsContext * sws_ctx;
    AVFrame *  av_frame;
    AVPacket * av_packet;
    AVCodecParserContext * parser;
} ffmpeg_client_ctx_t;

#endif //SHAREDT_FFMPEG_CLIENT_INTERFACE_H
