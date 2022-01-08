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
    return (avcodec_receive_frame(dec_ctx, frame) == 0) ? TRUE : FALSE;
}

rfbBool decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{
    int ret;

    if (!dec_ctx || !frame || !pkt) return FALSE;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret == AVERROR(EAGAIN)) {
        rfbErr("Error sending a packet for decoding, reason='Resource temporarily unavailable'\n");
        fetch_frame(dec_ctx, frame);  /* fetch frame to drain one frame, TODO: may need fetching multi frame ? */
        avcodec_send_packet(dec_ctx, pkt); /* resend the pkt, as we can't aford to discard one */
    } else if (ret < 0) {
        rfbErr("Error sending a packet for decoding\n");
        return FALSE;
    }

    return fetch_frame(dec_ctx, frame);
}

