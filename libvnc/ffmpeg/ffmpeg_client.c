#include <rfb/rfbclient.h>
#include <libswscale/swscale.h>

#include "ffmpeg_client.h"
#include "ffmpeg_interface.h"
#include "ffmpeg_client_interface.h"

static const AVCodec * codec   = NULL;
static AVCodecContext * codec_ctx   = NULL;
static struct SwsContext * sws_ctx  = NULL;
static AVFrame *  av_frame  = NULL;
static AVPacket * av_packet = NULL;
static AVCodecParserContext * parser = NULL;

static int writer_counter = 0;

static void
rfbDefaultLogStd(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fflush(stderr);
    va_end(args);
}

static void
rfbDefaultLogError(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);
    fflush(stderr);
    va_end(args);
}

rfbLogProc rfbLog=rfbDefaultLogStd;
rfbLogProc rfbErr=rfbDefaultLogError;

/*
 * TODO, if return FALSE. Exiting the program or consume the buffer and waiting for next loop.
 */
rfbBool
rfbSendRectEncodingFFMPEG(rfbClient* client,
                          rfbFramebufferUpdateRectHeader * rect)
{
    FFMPEG_HEADER_T av_header = { 0 };

    /* find the mpeg1video encoder */
    if (codec == NULL && (codec=avcodec_find_decoder_by_name(codec_name)) == NULL) {
        rfbErr("Codec '%s' not found\n", codec_name);
        return FALSE;
    }

    if (parser == NULL && ( parser = av_parser_init(codec->id)) == NULL)
    {
        rfbErr("parser not found\n");
        return FALSE;
    }

    if (av_packet == NULL && (av_packet=av_packet_alloc()) == NULL) {
        rfbErr("Failed to allocate packet for decoding\n");
        return FALSE;
    }


    if (codec_ctx == NULL) {
        if ((codec_ctx = avcodec_alloc_context3(codec)) == NULL) {
            rfbErr("Could not allocate video codec context\n");
            return FALSE;
        }

        codec_ctx->width = rect->r.w;
        codec_ctx->height = rect->r.h;
        if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
            fprintf(stderr, "Could not open codec\n");
            return FALSE;
        }
    }

    if (sws_ctx == NULL) {
        sws_ctx = sws_getContext(rect->r.w, rect->r.h, AV_PIX_FMT_YUV420P,
                                 rect->r.w, rect->r.h, AV_PIX_FMT_RGB32,
                                   SWS_BICUBIC, NULL,NULL,NULL);
    }

    /* Read header */
    if (!ReadFromRFBServer(client, (char *) av_header.header, sizeof(av_header)))
        return FALSE;
    av_header.HEADER.ffmpeg_body_len = rfbClientSwap32IfLE(av_header.HEADER.ffmpeg_body_len);

    if (!realloc_total_packet_buf(&av_packet_buf, av_header.HEADER.ffmpeg_body_len)) {
        rfbErr("Error to reallocate buffer, size=%d\n", av_header.HEADER.ffmpeg_body_len);
        return FALSE;
    }

    /* Read pakcet content */
    if (!ReadFromRFBServer(client, (char *) av_packet_buf._data, av_header.HEADER.ffmpeg_body_len)) {
        rfbErr("Failed to read data for data_len=%d\n", av_header.HEADER.ffmpeg_body_len);
        return FALSE;
    }
    av_packet_buf._size = av_header.HEADER.ffmpeg_body_len;

    if (av_parser_parse2(parser, codec_ctx, &av_packet->data, &av_packet->size,
                         av_packet_buf._data, (int) av_packet_buf._size,
                         AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0) < 0) {
        fprintf(stderr, "Error while parsing\n");
        return FALSE;
    }

    client->_available_frame = 0;

    /*
     * TODO, handle if width and height changed
     */
    if (rect->r.w == 0 ||  rect->r.h == 0 ||
        NULL == (av_frame=alloc_avframe(av_frame, rect->r.w, rect->r.h, AV_PIX_FMT_YUV420P))) {
        fprintf(stderr, "Could not allocate video frame, rect->r.w=%d, rect->r.h=%d\n",
                rect->r.w, rect->r.h);
        return FALSE;
    }

    rfbErr("Recevied packet size: %lu, av_packet_buf._size:%lu, av_frame->width:%d, av_frame->height:%d\n",
           av_packet->size, av_packet_buf._size, av_frame->width, av_frame->height);
    if (av_packet->size) {
        if (decode(codec_ctx, av_frame, av_packet) ||
                fetch_frame(codec_ctx, av_frame)) {
            client->_available_frame = 1;
            convert_to_avframeRGB32(sws_ctx, av_frame, (char * )client->frameBuffer,
                                    client->width, client->height);
/*
            char path[128] = {'\0'};
            sprintf(path,  "output_%d_.png", writer_counter++);
            write_RGB32_image(path, (unsigned char * )client->frameBuffer, client->width, client->height);
*/
            rfbLog("Recevied frame size linesize[0]: %lu, linesize[1]: %lu\n", av_frame->linesize[0], av_frame->linesize[1]);
        }
        else
            client->_available_frame = 0;
    }

    return TRUE;
}

