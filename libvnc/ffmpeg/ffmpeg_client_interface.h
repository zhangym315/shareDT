#ifndef SHAREDT_FFMPEG_CLIENT_INTERFACE_H
#define SHAREDT_FFMPEG_CLIENT_INTERFACE_H
#include <libavcodec/avcodec.h>
#include <rfb/rfb.h>

rfbBool fetch_frame(AVCodecContext *dec_ctx, AVFrame *frame);
rfbBool decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);

#endif //SHAREDT_FFMPEG_CLIENT_INTERFACE_H
