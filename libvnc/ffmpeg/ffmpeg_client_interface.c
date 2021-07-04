#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#include "ffmpeg_client_interface.h"

/*
 * Trying to receive frame
 * Return FALSE: no frame received
 * Return TRUE : frame returned
 */
rfbBool fetch_frame(AVCodecContext *dec_ctx, AVFrame *frame)
{
    int ret = avcodec_receive_frame(dec_ctx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return FALSE;
    } else if (ret < 0) {
        rfbErr("Error during decoding\n");
        return FALSE;
    } else {
        return TRUE;
    }
}

rfbBool decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{
    int ret;

    if (!dec_ctx || !frame || !pkt) return FALSE;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        rfbErr("Error sending a packet for decoding\n");
        return FALSE;
    }

    return fetch_frame(dec_ctx, frame);
}

