#ifndef SHAREDT_FFMPEGUTIL_H
#define SHAREDT_FFMPEGUTIL_H

extern "C" {
#include <libavcodec/avcodec.h>
};

int save_frame_as_jpeg(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo);

#endif //SHAREDT_FFMPEGUTIL_H
