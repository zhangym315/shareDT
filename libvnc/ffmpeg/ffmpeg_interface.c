#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>

#include "ffmpeg_interface.h"
#include "Size.h"

void release_total_packet_buf()
{
    if (av_packet_buf._capacity > 0 && av_packet_buf._data != NULL) {
        free(av_packet_buf._data);
        av_packet_buf._capacity = 0;
        av_packet_buf._size = 0;
    }
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
        atexit(release_total_packet_buf);
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
    packetBuf->_capacity = total_new_size;

    return TRUE;
}


AVCodecContext * openCodec(const char * codec_name, int w, int h)
{
    int ret;
    AVCodecContext * c;

    /* find the mpeg1video encoder */
    const AVCodec * codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) {
        rfbErr("Codec '%s' not found\n", codec_name);
        return NULL;
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        rfbErr("Could not allocate video codec context\n");
        return NULL;
    }

    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = w;
    c->height = h;

    /* frames per second */
    c->time_base = (AVRational){1, 25};
    c->framerate = (AVRational){25, 1};

    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if ((ret=avcodec_open2(c, codec, NULL)) < 0) {
        rfbErr("Could not open codec: %s\n", av_err2str(ret));
        return NULL;
    }

    return c;
}

AVPacket * get_packet(int size)
{
    AVPacket  * pkt = av_packet_alloc();
    if (!pkt)
        return NULL;

    return pkt;
}

struct SwsContext * get_yuv420_ctx(int w, int h, enum AVPixelFormat src_format)
{
    return sws_getContext(w, h, src_format,
                          w, h, AV_PIX_FMT_YUV420P,
                          SWS_BICUBIC, NULL,NULL,NULL);
}

void convert_to_avframeYUV420(struct SwsContext * sws_ctx, AVFrame *pict,
                           const char * data_frame, int w, int h)
{
    int data[8] = {(int) w * 3, 0, 0, 0, 0, 0, 0, 0};
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

    if (av_frame_get_buffer(frame, 0) < 0) {
        rfbErr("Could not allocate the video frame data\n");
        return NULL;
    }

    return frame;
}

