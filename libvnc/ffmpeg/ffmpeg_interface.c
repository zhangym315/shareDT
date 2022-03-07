#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <rfb/rfbproto.h>
#include "ffmpeg_interface.h"
#include "Size.h"

#include <png.h>

const char FFMPEG_HEADER_KEY[] = "FFMPEGHEADER";
const int  FFMPEG_HEADER_KEY_LEN = 12;
const int  FFMPEG_HEADER_LEN = sizeof(FFMPEG_HEADER_T);

const EncoderDecoderContext codecsContext[] = {
    /* -encodings,  en-codec name, de-codec name, pix-format,         ctx,  ffmpeg code,               */
    {"h263",        "h263",       "h263",         AV_PIX_FMT_YUV422P, NULL, rfbEncodingFFMPEG_H263      },
    {"x265_420",    "libx265",    "hevc",         AV_PIX_FMT_YUV420P, NULL, rfbEncodingFFMPEG_X265_420  },
    {"x265_422",    "libx265",    "hevc",         AV_PIX_FMT_YUV422P, NULL, rfbEncodingFFMPEG_X265_422  },
    {"x265_444",    "libx265",    "hevc",         AV_PIX_FMT_YUV444P, NULL, rfbEncodingFFMPEG_X265_444  },
    {"x265_GBRP",   "libx265",    "hevc",         AV_PIX_FMT_GBRP,    NULL, rfbEncodingFFMPEG_X265_GBRP },
    {"mpeg2_420",   "mpeg2video", "mpeg2video",   AV_PIX_FMT_YUV420P, NULL, rfbEncodingFFMPEG_MPEG2_420 },
    {"mpeg2_422",   "mpeg2video", "mpeg2video",   AV_PIX_FMT_YUV422P, NULL, rfbEncondigFFMPEG_MPEG2_422 },
    {"png_RGB",     "png",        "png",          AV_PIX_FMT_RGB24,   NULL, rfbEncondigFFMPEG_PNG_RGB   },
    {"ppg_RGB",     "ppm",        "ppm",          AV_PIX_FMT_RGB24,   NULL, rfbEncondigFFMPEG_PPG_RGB   },
    {"mpeg4_420",   "mpeg4",      "mpeg4",        AV_PIX_FMT_YUV420P, NULL, rfbEncondigFFMPEG_MPEG4_420 },
    {NULL, NULL, NULL, AV_PIX_FMT_NONE, NULL}
};

const EncoderDecoderContext * getEncoderDecoderContextByName(const char * encoding)
{
    const EncoderDecoderContext * ptr = codecsContext;
    size_t len = strlen(encoding);
    while (ptr->encoding_name) {
        if (strncasecmp(encoding, ptr->encoding_name, len) == 0)
            return ptr;
        ptr++;
    }
    return NULL;
}

/*
 * @realloc_size
 * Allocate enough buffer to packetBuf
 * The real allocated buffer is normally (size*2) until it reached to MAX_AVPACKET_BUFFER(128MB)
 * If it reached to MAX_AVPACKET_BUFFER, it will allocate the required size of buffer.
 *
 * size: required total size
 *
 */
rfbBool realloc_total_packet_buf(AVPacketBuf * packetBuf, size_t size)
{
    size_t total_new_size;

    /* initial allocate */
    if (packetBuf->_capacity == 0 && packetBuf->_data == NULL) {
        total_new_size = size * 2;
        if ((packetBuf->_data = malloc(total_new_size)) == NULL) {
            rfbErr("Error to allocate buffer with size=%ul\n", total_new_size);
            return FALSE;
        }
        packetBuf->_capacity = total_new_size;
        memset(packetBuf->_data, 0, packetBuf->_capacity);
//        atexit(release_total_packet_buf, packetBuf);  // TODO free buffer !!
        return TRUE;
    }

    if (packetBuf->_capacity >= size) {
        return TRUE;
    }

    total_new_size = (size*2 > MAX_AVPACKET_BUFFER*MEGABYTE ) ? size : size * 2;
    if ((packetBuf->_data = realloc(packetBuf->_data, total_new_size)) == NULL) {
        rfbErr("Error to allocate buffer with size=%ul\n", total_new_size);
        return FALSE;
    }
    memset(packetBuf->_data, 0, packetBuf->_capacity);
    packetBuf->_capacity = total_new_size;

    return TRUE;
}


AVCodecContext * openCodec(const EncoderDecoderContext * ctx, int w, int h)
{
    int ret;
    AVCodecContext * c;

    /* find the mpeg1video encoder */
    const AVCodec * codec = avcodec_find_encoder_by_name(ctx->codec_name);
    if (!codec) {
        rfbErr("Codec '%s' not found\n", ctx->codec_name);
        return NULL;
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        rfbErr("Could not allocate video codec context\n");
        return NULL;
    }

    c->width = w;
    c->height = h;

    /* frames per second */
    c->time_base = (AVRational){1, 60};
    c->framerate = (AVRational){60, 1};

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = 1;
    c->max_b_frames = 1;
    c->pix_fmt = ctx->pix_format;

    if ((ret=avcodec_open2(c, codec, NULL)) < 0) {
        rfbErr("Could not open codec: %s\n", av_err2str(ret));
        return NULL;
    }

    rfbLog("Opened Codec with codec_name=%s pix_format=%d\n", ctx->codec_name, ctx->pix_format);
    return c;
}

struct SwsContext * get_SwsContext(int w, int h, enum AVPixelFormat src_format, enum AVPixelFormat dst_format)
{
    return sws_getContext(w, h, src_format,
                          w, h, dst_format,
                          SWS_BICUBIC, NULL,NULL,NULL);
}

void convert_to_avframe(struct SwsContext * sws_ctx, AVFrame *pict,
                           const char * data_frame, int w, int h)
{
    int data[8] = {(int) w * 4, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t *const srcSlice[8] = { (uint8_t *)data_frame, 0, 0, 0, 0, 0, 0, 0 };

    sws_scale ( sws_ctx, srcSlice, data, 0,
                h, pict->data, pict->linesize );
}

void convert_to_avframeRGB32(struct SwsContext * sws_ctx, AVFrame * srcFrame,
                             char * data_frame, int w, int h)
{
    int data[8] = {(int) w * 4, 0, 0, 0, 0, 0, 0, 0};
    uint8_t * srcSlice[8] = { (uint8_t *) data_frame, 0, 0, 0, 0, 0, 0, 0 };

    sws_scale ( sws_ctx, (const uint8_t *const *)(srcFrame->data), (const int *)(srcFrame->linesize), 0,
                h, srcSlice, data );

}

AVFrame * alloc_avframe(AVFrame * src, int w, int h, enum AVPixelFormat format)
{
    /* if src remains the same, no need to re-alloca */
    if (src !=NULL && (src->width != w || src->height != h || src->format != format)) {
        av_frame_free(&src);
    } else if (src !=NULL){
        return src;
    }

    AVFrame * frame = av_frame_alloc();
    if (!frame) {
        rfbErr("Could not allocate video frame\n");
        return NULL;
    }
    frame->format = format;
    frame->width  = w;
    frame->height = h;

    frame->pts = 0;
    if (av_frame_get_buffer(frame, 0) < 0) {
        rfbErr("Could not allocate the video frame data\n");
        return NULL;
    }

    return frame;
}

void write_RGB32_image(const char * path, unsigned char *buffer, size_t w, size_t h)
{
    FILE *fp = fopen(path, "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(png,
                 info,
                 w, h,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    for ( int i=0 ; i<h ; i++) {
        png_write_row(png, (png_bytep)(buffer + i*w*4));
    }

    png_write_end(png, NULL);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

}

void write_YUV_image(char  * path, AVFrame * av_frame)
{
    struct SwsContext * sws_ctx;
    AVFrame * rgbFrame;
    if ((sws_ctx = sws_getContext(av_frame->width, av_frame->height, av_frame->format,
                                       av_frame->width, av_frame->height, AV_PIX_FMT_RGB32,
                                       SWS_BICUBIC, NULL,NULL,NULL)) == NULL ) {
        rfbErr("Could get sws_getContext\n");
        return;
    }
    if (NULL == (rgbFrame=alloc_avframe(NULL, av_frame->width, av_frame->height, AV_PIX_FMT_RGB32))) {
        rfbErr("Could not allocate video frame, rect->r.w=%d, rect->r.h=%d\n",
                av_frame->width, av_frame->height);
        return;
    }

    convert_to_avframeRGB32(sws_ctx, av_frame, (char *) rgbFrame->data[0], av_frame->width, av_frame->height);

    write_RGB32_image(path, rgbFrame->data[0], av_frame->width, av_frame->height);
    av_frame_free(&rgbFrame);
}
