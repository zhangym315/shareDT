#include <rfb/rfbclient.h>
#include <libswscale/swscale.h>

#include "ffmpeg_client.h"
#include "ffmpeg_interface.h"
#include "ffmpeg_client_interface.h"

static uint64_t total_received_bytes = 0;

//static writer_counter = 0;
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

static ffmpeg_client_ctx_t * get_client_ctxs(int w, int h)
{
    ffmpeg_client_ctx_t * ret   = NULL;

    ret = (ffmpeg_client_ctx_t * ) malloc(sizeof(ffmpeg_client_ctx_t));
    if (ret == NULL) {
        rfbErr("Cannot allocate for ffmpeg_client_ctx_t.\n");
        return NULL;
    }
    memset(ret, 0, sizeof(ffmpeg_client_ctx_t));

    const char * codec_name = strcmp("libx265", current_codec->codec_name) == 0 ?
                                "hevc" : current_codec->codec_name;

    if ((ret->codec=avcodec_find_decoder_by_name(codec_name)) == NULL) {
        rfbErr("Codec '%s' not found\n", codec_name);
        goto failed;
    } else
    rfbErr("Codec '%s' indeed found\n", codec_name);

    if ((ret->parser = av_parser_init(ret->codec->id)) == NULL)
    {
        rfbErr("parser not found\n");
        goto failed;
    }

    if ((ret->av_packet=av_packet_alloc()) == NULL) {
        rfbErr("Failed to allocate packet for decoding\n");
        goto failed;
    }


    if ((ret->codec_ctx = avcodec_alloc_context3(ret->codec)) == NULL) {
        rfbErr("Could not allocate video codec context\n");
        goto failed;
    }

    ret->codec_ctx->width = w;
    ret->codec_ctx->height = h;
    if (avcodec_open2(ret->codec_ctx, ret->codec, NULL) < 0) {
        rfbErr("Could not open codec\n");
        goto failed;
    }

    if ((ret->sws_ctx = sws_getContext(w, h, current_codec->pix_format,
                                       w, h, AV_PIX_FMT_RGB32,
                                       SWS_BICUBIC, NULL,NULL,NULL)) == NULL ) {
        rfbErr("Could get sws_getContext\n");
        goto failed;
    }

    return ret;

failed:
    free(ret);
    return NULL;
}

/*
 * TODO, if return FALSE. Exiting the program or consume the buffer and waiting for next loop.
 */
rfbBool
rfbReceiveRectEncodingFFMPEG(rfbClient* client,
                          rfbFramebufferUpdateRectHeader * rect)
{
    FFMPEG_HEADER_T av_header = { 0 };

    if (client->_ffmpeg_decoder == NULL) {
        client->_ffmpeg_decoder = get_client_ctxs(rect->r.w, rect->r.h);
        if (client->_ffmpeg_decoder == NULL) {
            rfbErr("Error to get client ffmpeg ctxs\n");
            return FALSE;
        }
    }

    ffmpeg_client_ctx_t * decoder_ctx = (ffmpeg_client_ctx_t *) client->_ffmpeg_decoder;

    /* Read header */
    if (!ReadFromRFBServer(client, (char *) av_header.header, sizeof(av_header)))
        return FALSE;
    av_header.HEADER.ffmpeg_body_len = rfbClientSwap32IfLE(av_header.HEADER.ffmpeg_body_len);

//    rfbLog("%s received data_size=%d\n", get_current_time_string(), av_header.HEADER.ffmpeg_body_len + sizeof(av_header));

    /* No body data, just return */
    if (av_header.HEADER.ffmpeg_body_len == 0) {
        client->_available_frame = 0;
        return TRUE;
    }

    /* make sure buffer is big enough */
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

    int parsed;
    uint8_t *data = av_packet_buf._data;
    size_t   data_size = av_packet_buf._size;
    client->_available_frame = 0;
    /*
     * Send packet to decode.
     * If multi-frames returned, send the last frame.
     */
    while(data_size > 0) {
        parsed = av_parser_parse2(decoder_ctx->parser,
                                  decoder_ctx->codec_ctx,
                                  &decoder_ctx->av_packet->data,
                                  &decoder_ctx->av_packet->size,
                                  data, (int) data_size,
                                  AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

        if (parsed < 0) {
            fprintf(stderr, "Error while parsing\n");
            return FALSE;
        }
        data      += parsed;
        data_size -= parsed;

        total_received_bytes += av_packet_buf._size + sizeof(av_header);

        /* get av_frame */
        if (rect->r.w == 0 ||  rect->r.h == 0 ||
            NULL == (decoder_ctx->av_frame=alloc_avframe(decoder_ctx->av_frame, rect->r.w,
                                                         rect->r.h, current_codec->pix_format))) {
            fprintf(stderr, "Could not allocate video frame, rect->r.w=%d, rect->r.h=%d\n",
                    rect->r.w, rect->r.h);
            return FALSE;
        }

        if (decoder_ctx->av_packet->size &&
            (decode(decoder_ctx->codec_ctx,
                    decoder_ctx->av_frame,
                    decoder_ctx->av_packet) ||
            fetch_frame(decoder_ctx->codec_ctx,
                        decoder_ctx->av_frame))) {
            client->_available_frame = 1;
/* TESTING
            char path[128] = {'\0'};
            sprintf(path,  "receive_output_%d_.png", writer_counter++);
            write_YUV_image(path, decoder_ctx->av_frame);
            writer_counter++;
            printf("%s get new frame with frame_number=%d\n", get_current_time_string(), writer_counter);
            memset(client->frameBuffer, 0, 1000);
*/
            convert_to_avframeRGB32(decoder_ctx->sws_ctx, decoder_ctx->av_frame,
                                    (char * ) client->frameBuffer,
                                    client->width, client->height);

/*            rfbLog("%s Recevied frame packet_size=%lu, frame_pts=%llu, total_received_bytes=%llu\n",
                   get_current_time_string(),
                   decoder_ctx->av_packet->size,
                   decoder_ctx->av_frame->pts,
                   total_received_bytes);
*/
        }
    }

    return TRUE;
}

