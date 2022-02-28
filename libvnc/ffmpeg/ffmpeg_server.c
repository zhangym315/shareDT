#include "ffmpeg_server.h"
#include "ffmpeg_server_interface.h"
#include "ffmpeg_interface.h"
#include "TimeUtil.h"

static int writer_counter = 0;

static ffmpeg_server_ctx_t * get_server_ctxs(int w, int h)
{
    ffmpeg_server_ctx_t * ret   = NULL;
    struct SwsContext * sws_ctx = NULL;
    AVCodecContext * codec_ctx  = NULL;
    AVFrame *  av_frame  = NULL;

    if ((codec_ctx=openCodec(current_codec->codec_name, w, h)) == NULL) {
        rfbErr("Failed to open codec in ffmpeg\n");
        return ret;
    }

    if ((av_frame=alloc_avframe(av_frame, w, h, current_codec->pix_format)) == NULL) {
        rfbErr("Failed to alloac avframe for width=%d height=%d format=%d\n", w, h, current_codec->pix_format);
        return ret;
    }
    rfbLog("frame size: w=%d, h=%d, av_frame.size:%lu\n", w, h, av_frame->buf[0]->size);

    if ((sws_ctx=get_yuv420_ctx(w, h, AV_PIX_FMT_RGB32)) == NULL) {
        fprintf(stderr, "Failed to alloac avframe for width=%d height=%d format=%d\n", w, h, AV_PIX_FMT_RGB32);
        return ret;
    }

    ret = (ffmpeg_server_ctx_t *) malloc (sizeof(ffmpeg_server_ctx_t)); /* TODO: needs to release while clientPtr is freed */

    if (ret == NULL) return NULL;

    ret->codec_ctx = codec_ctx;
    ret->av_frame  = av_frame;
    ret->sws_ctx   = sws_ctx;

    return ret;
}

rfbBool
rfbSendRectEncodingFFMPEG(rfbClientPtr cl,
                          int x,
                          int y,
                          int w,
                          int h)
{

    if (cl->sock < 0) return FALSE;

    /* initialize */
    if (cl->ffmpeg_encoder == NULL) {
        cl->ffmpeg_encoder = get_server_ctxs(w, h);
    }

    ffmpeg_server_ctx_t * server_ctx = (ffmpeg_server_ctx_t *) cl->ffmpeg_encoder;

    if (x || y) {
        rfbErr("Start point is not at (0, 0), current is with x=%d, y=%d\n", x, y);
        return FALSE;
    }

    rfbFramebufferUpdateRectHeader rect;
    int bytesPerLine = w * 4;

    /* Flush the buffer to guarantee correct alignment for translateFn(). */
    if (cl->ublen > 0) {
        if (!rfbSendUpdateBuf(cl))
            return FALSE;
    }

    rect.r.x = Swap16IfLE(x);
    rect.r.y = Swap16IfLE(y);
    rect.r.w = Swap16IfLE(w);
    rect.r.h = Swap16IfLE(h);
    rect.encoding = Swap32IfLE(rfbEncodingFFMPEG);
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
    convert_to_avframeYUV420(server_ctx->sws_ctx, server_ctx->av_frame, cl->scaledScreen->frameBuffer, w, h);

    AVPacketBuf * packet_buf = encode(server_ctx->codec_ctx, server_ctx->av_frame);

    if (!packet_buf) {
        rfbErr("encoded avframe, receive zero packet\n");
        return TRUE;
    }

    rfbStatRecordEncodingSent(cl, rfbEncodingFFMPEG, packet_buf->_size,
                              sz_rfbFramebufferUpdateRectHeader + bytesPerLine * h);

    if (rfbWriteExact(cl, (const char *) packet_buf->_data, packet_buf->_size) < 0) {
        rfbLogPerror("rfbSendUpdateBuf: write");
        rfbCloseClient(cl);
        return FALSE;
    }

    rfbLog("%s Packet write_size=%d, total_size=%d frame_pks=%d\n",
           get_current_time_string(), packet_buf->_size, cl->bytesSent, server_ctx->av_frame->pts);

    packet_buf->_size = 0;

    return TRUE;
}
