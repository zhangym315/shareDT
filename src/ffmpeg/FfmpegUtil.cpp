
#include "FfmpegUtil.h"
#if 0
int save_frame_as_jpeg(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo) {
    int ret;
    const AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!jpegCodec) {
        return -1;
    }

    AVCodecContext *jpegContext = avcodec_alloc_context3(jpegCodec);
    if (!jpegContext) {
        return -1;
    }

    jpegContext->pix_fmt = pCodecCtx->pix_fmt;
    jpegContext->height = pFrame->height;
    jpegContext->width = pFrame->width;
    if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
        return -1;
    }

    FILE *JPEGFile;
    char JPEGFName[256];
    AVPacket * packet = av_packet_alloc();
    sprintf(JPEGFName, "dvr-%06d.jpg", FrameNo);
    JPEGFile = fopen(JPEGFName, "wb");

    int gotFrame;

    while ((ret=avcodec_receive_packet(pCodecCtx, packet)) >= 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write packet %3 (size=%5d)\n", packet->pts, packet->size);
        fwrite(packet->data, 1, packet->size, JPEGFile);
        av_packet_unref(packet);
    }

    fclose(JPEGFile);

//    av_free_packet(packet);
    avcodec_close(jpegContext);

    return 0;
}
#endif