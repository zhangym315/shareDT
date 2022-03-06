#ifndef SHAREDT_FFMPEG_INTERFACE_H
#define SHAREDT_FFMPEG_INTERFACE_H

#include <libavutil/pixfmt.h>
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

typedef struct {
    const char * encoding_name;      /* used to negotiate between server and client */
    const char * codec_name;         /* ffmpeg codec name                           */
    const char * decodec_name;       /* ffmpeg codec name                           */
    enum AVPixelFormat pix_format;   /* ffmpeg pix format                           */
    void * ctx;                      /* context for codec of client and server      */
    int  code;                       /* encoding code for communication between c s */
} EncoderDecoderContext;

extern const EncoderDecoderContext * getEncoderDecoderContextByName(const char * encoding);
extern AVCodecContext * openCodec(const EncoderDecoderContext * ctx, int w, int h);

extern AVFrame * alloc_avframe(AVFrame * src, int w, int h, enum AVPixelFormat format);

extern struct SwsContext * get_SwsContext(int w, int h, enum AVPixelFormat src_format, enum AVPixelFormat dst_format);
extern void convert_to_avframe(struct SwsContext * sws_ctx, AVFrame *pict,
                                     const char * data_frame, int w, int h);
extern void convert_to_avframeRGB32(struct SwsContext * sws_ctx, AVFrame * srcFrame,
                                     char * data_frame, int w, int h);
extern rfbBool realloc_total_packet_buf(AVPacketBuf * packetBuf, size_t size);
extern void write_RGB32_image(const char * path, unsigned char *buffer, size_t w, size_t h);
extern void write_YUV_image(char  * path, AVFrame * av_frame);

typedef union {
    unsigned char header[16];
    struct {
        char FFMPEG_HEADER[12];
        uint32_t ffmpeg_body_len;   // max 4 GB
    } HEADER;
} FFMPEG_HEADER_T;

extern const char FFMPEG_HEADER_KEY[];
extern const int  FFMPEG_HEADER_KEY_LEN;
extern const int  FFMPEG_HEADER_LEN;
extern const EncoderDecoderContext codecsContext[];

#endif //SHAREDT_FFMPEG_INTERFACE_H
