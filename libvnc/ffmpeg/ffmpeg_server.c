#include "ffmpeg_server.h"
#include "ffmpeg_server_interface.h"
#include "ffmpeg_interface.h"
#include "TimeUtil.h"

static int writer_counter = 0;

static ffmpeg_server_ctx_t * get_server_ctxs(int w, int h, int encoding)
{
    ffmpeg_server_ctx_t * ret   = NULL;
    struct SwsContext * sws_ctx = NULL;
    AVCodecContext * codec_ctx  = NULL;
    AVFrame *  av_frame  = NULL;

    if ((codec_ctx=openCodec(&codecsContext[encoding - rfbEncodingFFMPEG_H263], w, h)) == NULL) {
        rfbErr("Failed to open codec in ffmpeg\n");
        return ret;
    }

    if ((av_frame=alloc_avframe(av_frame, w, h, codecsContext[encoding - rfbEncodingFFMPEG_H263].pix_format)) == NULL) {
        rfbErr("Failed to alloac avframe for width=%d height=%d format=%d\n", w, h, codecsContext[encoding - rfbEncodingFFMPEG_H263].pix_format);
        return ret;
    }
    rfbLog("frame size: w=%d, h=%d, av_frame.size:%lu\n", w, h, av_frame->buf[0]->size);

    if ((sws_ctx=get_SwsContext(w, h, AV_PIX_FMT_RGB32, codecsContext[encoding - rfbEncodingFFMPEG_H263].pix_format)) == NULL) {
        fprintf(stderr, "Failed to alloac avframe for width=%d height=%d format=%d\n", w, h, AV_PIX_FMT_RGB32);
        return ret;
    }

    ret = (ffmpeg_server_ctx_t *) malloc (sizeof(ffmpeg_server_ctx_t)); /* TODO: needs to release while clientPtr is freed */

    if (ret == NULL) return NULL;

    ret->codec_ctx = codec_ctx;
    ret->av_frame  = av_frame;
    ret->sws_ctx   = sws_ctx;

    ret->buf._size = 0;
    ret->buf._capacity = 0;
    ret->buf._data = NULL;

    return ret;
}

rfbBool
rfbSendRectEncodingFFMPEG(rfbClientPtr cl)
{

    if (cl->sock < 0) return FALSE;

    int w = cl->screen->width;
    int h = cl->screen->height;

    int encoding = cl->preferredEncoding;
    /* initialize */
    if (cl->ffmpeg_encoder == NULL) {
        cl->ffmpeg_encoder = get_server_ctxs(w, h, encoding);
    }

    ffmpeg_server_ctx_t * server_ctx = (ffmpeg_server_ctx_t *) cl->ffmpeg_encoder;

    rfbFramebufferUpdateRectHeader rect;
    int bytesPerLine = w * 4;
    AVPacketBuf * output_buf = &server_ctx->buf;

    /* Flush the buffer to guarantee correct alignment for translateFn(). */
    if (cl->ublen > 0) {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    /*
     * send the whole screen update
     * start point is from (0, 0)
     */
    rect.r.x = Swap16IfLE(0);
    rect.r.y = Swap16IfLE(0);
    rect.r.w = Swap16IfLE(w);
    rect.r.h = Swap16IfLE(h);
    rect.encoding = Swap32IfLE(encoding);
    memcpy(&cl->updateBuf[cl->ublen], (char *)&rect, sz_rfbFramebufferUpdateRectHeader);
    cl->ublen += sz_rfbFramebufferUpdateRectHeader;

    if (!rfbSendUpdateBuf(cl))
        return FALSE;

/*    char path[128] = {'\0'};
    sprintf(path,  "output_%d_.png", writer_counter++);
    write_RGB32_image(path, cl->scaledScreen->frameBuffer, w, h);
    rfbErr("write to output file:%s\n", path);
*/

    /* encode avframe */
    server_ctx->av_frame->pts++;
    convert_to_avframe(server_ctx->sws_ctx, server_ctx->av_frame, cl->scaledScreen->frameBuffer, w, h);

    if (!encode(server_ctx->codec_ctx, server_ctx->av_frame, output_buf)) {
        rfbErr("encoded avframe, receive zero packet\n");
        return TRUE;
    }

    rfbStatRecordEncodingSent(cl, encoding, (int)output_buf->_size,
                              sz_rfbFramebufferUpdateRectHeader + bytesPerLine * h);

    if (rfbWriteExact(cl, (const char *) output_buf->_data, (int)output_buf->_size) < 0) {
        rfbLogPerror("rfbSendUpdateBuf: write");
        rfbCloseClient(cl);
        return FALSE;
    }

//    rfbLog("%s Packet write_size=%d, total_size=%d frame_pks=%d\n",
//           get_current_time_string(), output_buf->_size, cl->bytesSent, server_ctx->av_frame->pts);

    output_buf->_size = 0;

    return TRUE;
}
