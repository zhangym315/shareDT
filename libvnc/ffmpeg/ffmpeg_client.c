#include <rfb/rfbclient.h>
#include <libswscale/swscale.h>

#include "ffmpeg_client.h"
#include "ffmpeg_interface.h"
#include "ffmpeg_client_interface.h"

static writer_counter = 0;
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

rfbLogProc rfbLogInternal=rfbDefaultLogStd;
rfbLogProc rfbErrInternal=rfbDefaultLogError;

static ffmpeg_client_ctx_t * get_client_ctxs(int w, int h, uint32_t encode)
{
    ffmpeg_client_ctx_t * ret   = NULL;
    const EncoderDecoderContext * ctx = &codecsContext[encode - rfbEncodingFFMPEG_H264];

    ret = (ffmpeg_client_ctx_t * ) malloc(sizeof(ffmpeg_client_ctx_t));
    if (ret == NULL) {
        rfbErrInternal("Cannot allocate for ffmpeg_client_ctx_t.\n");
        return NULL;
    }
    memset(ret, 0, sizeof(ffmpeg_client_ctx_t));

    ret->codec = (strcmp(ctx->codec_name, "libx264rgb") == 0) ? avcodec_find_decoder(AV_CODEC_ID_H264)
                                                              : avcodec_find_decoder_by_name(ctx->codec_name);
    if (ret->codec == NULL) {
        rfbErrInternal("Codec '%s' not found\n", ctx->decodec_name);
        goto failed;
    } else
        rfbLogInternal("Codec '%s' found\n", ctx->decodec_name);

    if ((ret->parser = av_parser_init(ret->codec->id)) == NULL)
    {
        rfbErrInternal("parser not found\n");
        goto failed;
    }

    if ((ret->av_packet=av_packet_alloc()) == NULL) {
        rfbErrInternal("Failed to allocate packet for decoding\n");
        goto failed;
    }


    if ((ret->codec_ctx = avcodec_alloc_context3(ret->codec)) == NULL) {
        rfbErrInternal("Could not allocate video codec context\n");
        goto failed;
    }

    ret->codec_ctx->width = w;
    ret->codec_ctx->height = h;
    if (avcodec_open2(ret->codec_ctx, ret->codec, NULL) < 0) {
        rfbErrInternal("Could not open codec\n");
        goto failed;
    }

    if ((ret->sws_ctx = sws_getContext(w, h, ctx->pix_format,
                                       w, h, AV_PIX_FMT_RGB32,
                                       SWS_BICUBIC, NULL,NULL,NULL)) == NULL ) {
        rfbErrInternal("Could get sws_getContext\n");
        goto failed;
    }

    ret->buf._size = 0;
    ret->buf._capacity = 0;
    ret->buf._data = NULL;

    ret->total_received_bytes = 0;

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
                             rfbFramebufferUpdateRectHeader * rect,
                             uint32_t encode)
{
    FFMPEG_HEADER_T av_header = { 0 };

    if (client->_ffmpeg_decoder == NULL) {
        client->_ffmpeg_decoder = get_client_ctxs(rect->r.w, rect->r.h, encode);
        if (client->_ffmpeg_decoder == NULL) {
            rfbErrInternal("Error to get client ffmpeg ctxs\n");
            return FALSE;
        }
    }

    ffmpeg_client_ctx_t * decoder_ctx = (ffmpeg_client_ctx_t *) client->_ffmpeg_decoder;
    AVPacketBuf * cl_av_packet_buf = &decoder_ctx->buf;

    /* Read header */
    if (!ReadFromRFBServer(client, (char *) av_header.header, sizeof(av_header)))
        return FALSE;
    av_header.HEADER.ffmpeg_body_len = rfbClientSwap32IfLE(av_header.HEADER.ffmpeg_body_len);

//    rfbLogInternal("%s received data_size=%d\n", get_current_time_string(), av_header.HEADER.ffmpeg_body_len + sizeof(av_header));

    /* No body data, just return */
    if (av_header.HEADER.ffmpeg_body_len == 0) {
        client->_available_frame = 0;
        return TRUE;
    }

    /* make sure buffer is big enough */
    if (!realloc_total_packet_buf(cl_av_packet_buf, av_header.HEADER.ffmpeg_body_len)) {
        rfbErrInternal("Error to reallocate buffer, size=%d\n", av_header.HEADER.ffmpeg_body_len);
        return FALSE;
    }

    /* Read pakcet content */
    if (!ReadFromRFBServer(client, (char *) cl_av_packet_buf->_data, av_header.HEADER.ffmpeg_body_len)) {
        rfbErrInternal("Failed to read data for data_len=%d\n", av_header.HEADER.ffmpeg_body_len);
        return FALSE;
    }
    cl_av_packet_buf->_size = av_header.HEADER.ffmpeg_body_len;

    int parsed;
    uint8_t *data = cl_av_packet_buf->_data;
    size_t   data_size = cl_av_packet_buf->_size;
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

        decoder_ctx->total_received_bytes += cl_av_packet_buf->_size + sizeof(av_header);

        /* get av_frame */
        if (rect->r.w == 0 ||  rect->r.h == 0 ||
            NULL == (decoder_ctx->av_frame=alloc_avframe(decoder_ctx->av_frame, rect->r.w, rect->r.h,
                                                         codecsContext[encode - rfbEncodingFFMPEG_H264].pix_format))) {
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
*/

            convert_to_avframeRGB32(&(decoder_ctx->sws_ctx), decoder_ctx->av_frame,
                                    (char * ) client->frameBuffer,
                                    client->width, client->height);
/*
            rfbLogInternal("%s Recevied frame packet_size=%lu, frame_pts=%llu, decoder_ctx->total_received_bytes=%llu\n",
                   get_current_time_string(),
                   decoder_ctx->av_packet->size,
                   decoder_ctx->av_frame->pts,
                   decoder_ctx->total_received_bytes);
*/
        }
    }

    return TRUE;
}

