#include "ffmpeg_server.h"
#include "ffmpeg_server_interface.h"
#include "ffmpeg_interface.h"

static AVCodecContext * codec_ctx   = NULL;
static struct SwsContext * sws_ctx  = NULL;
static AVFrame *  av_frame  = NULL;
static int writer_counter = 0;
rfbBool
rfbSendRectEncodingFFMPEG(rfbClientPtr cl,
                          int x,
                          int y,
                          int w,
                          int h)
{

    if (cl->sock < 0) return FALSE;

    if (codec_ctx == NULL && (codec_ctx=openCodec(codec_name, w, h)) == NULL) {
        rfbErr("Failed to open codec in ffmpeg\n");
        return FALSE;
    }

    if (av_frame == NULL && (av_frame=alloc_avframe(av_frame, w, h, AV_PIX_FMT_YUV420P)) == NULL) {
        rfbErr("Failed to alloac avframe for width=%d height=%d format=%d\n", w, h, AV_PIX_FMT_YUV420P);
        return FALSE;
    }
    rfbErr("frame size: w=%d, h=%d, av_frame.size:%lu\n", w, h, av_frame->buf[0]->size);

    if (sws_ctx == NULL && (sws_ctx=get_yuv420_ctx(w, h, AV_PIX_FMT_RGB32)) == NULL) {
        fprintf(stderr, "Failed to alloac avframe for width=%d height=%d format=%d\n", w, h, AV_PIX_FMT_RGB32);
        return FALSE;
    }

    if (x || y) {
        rfbErr("Start point is not at (0, 0), current is with x=%d, y=%d\n", x, y);
        return FALSE;
    }

    rfbFramebufferUpdateRectHeader rect;
    int bytesPerLine = w * (cl->format.bitsPerPixel / 8);

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

    /* encode avframe */
//    memcpy(av_frame->data[0], cl->scaledScreen->frameBuffer, w*h*3);
    convert_to_avframeYUV420(sws_ctx, av_frame, cl->scaledScreen->frameBuffer, w, h);
    AVPacketBuf * packet_buf = encode(codec_ctx, av_frame);

/*
 * char path[128] = {'\0'};
    sprintf(path,  "output_%d_.png", writer_counter++);
    write_RGB32_image(path, (unsigned char * ) cl->scaledScreen->frameBuffer, w, h);
*/

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
rfbLog("Packet write: %d\n", packet_buf->_size);
    packet_buf->_size = 0;

    return TRUE;
}
