#ifndef SHAREDT_FFMPEG_INTERFACE_H
#define SHAREDT_FFMPEG_INTERFACE_H

#include <libavcodec/avcodec.h>
#include <rfb/rfb.h>

/*
 * A buffer that stores the total packets of a frame
 * Used to be the returned total packet buffer
 */
typedef struct {
    size_t   _size;        /* current valid size in the data */
    size_t   _capacity;    /* total alloc buffer size        */
    unsigned char * _data; /* buffer                         */
} AVPacketBuf;

extern AVCodecContext * openCodec(const char * codec_name, int w, int h);

extern AVFrame * alloc_avframe(AVFrame * src, int w, int h, enum AVPixelFormat format);

extern struct SwsContext * get_yuv420_ctx(int w, int h, enum AVPixelFormat src_format);
extern void convert_to_avframeYUV420(struct SwsContext * sws_ctx, AVFrame *pict,
                                     const char * data_frame, int w, int h);
extern void convert_to_avframeRGB32(struct SwsContext * sws_ctx, AVFrame * srcFrame,
                                     char * data_frame, int w, int h);
extern rfbBool realloc_total_packet_buf(AVPacketBuf * packetBuf, size_t size);

typedef union {
    unsigned char header[16];
    struct {
        char FFMPEG_HEADER[12];
        uint32_t ffmpeg_body_len;   // max 4 GB
    } HEADER;
} FFMPEG_HEADER_T;

const static char * FFMPEG_HEADER_KEY = "FFMPEGHEADER";
const static int FFMPEG_HEADER_KEY_LEN = 12;
const static int FFMPEG_HEADER_LEN = sizeof(FFMPEG_HEADER_T);
static AVPacketBuf av_packet_buf = { 0 };

static const char * codec_name  = "mpeg1video";

#endif //SHAREDT_FFMPEG_INTERFACE_H
