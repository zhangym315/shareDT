#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

#include "ffmpeg_server_interface.h"

rfbBool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacketBuf * out_buffer)
{
    int ret;
    AVPacket pkt = { 0 };

    if (!enc_ctx || !frame ) return FALSE;

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        rfbErr("Error sending a frame for encoding ret=%d\n", ret);
        return FALSE;
    }

    if (out_buffer->_size > 0) {
        rfbLog("Found current packet buffer size is not zero, out_buffer->_size=%ul\n", out_buffer->_size);
    }

    /*
     * Fill header, which is of following structure
     * "FFMPEGHEADER"|  bodylen    |    packet_body...
     * |--- 12 bytes-|-- 4 bytes --|---- body length ...
     *
     */
    if (!realloc_total_packet_buf(out_buffer, FFMPEG_HEADER_LEN)) {
        rfbErr("Error to allocate buffer size for header len=%d\n", FFMPEG_HEADER_LEN);
        return FALSE;
    }
    FFMPEG_HEADER_T * header = (FFMPEG_HEADER_T *) out_buffer->_data;
    memcpy(header->HEADER.FFMPEG_HEADER, FFMPEG_HEADER_KEY, FFMPEG_HEADER_KEY_LEN);
    out_buffer->_size += FFMPEG_HEADER_LEN;

    /*
     * Receive packet from ffmpeg encoder, and memcpy to out_buffer, which will be sent to client
     */
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, &pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            rfbErr("Error during encoding\n");
            break;
        }

        /* check if memory is big enough */
        if (((out_buffer->_capacity - out_buffer->_size) < pkt.size) ) {
            if (!realloc_total_packet_buf(out_buffer, pkt.size+out_buffer->_size)) {
                rfbErr("Error to allocate buffer size for total packets\n");
                break;
            }
            // header changed
            header = (FFMPEG_HEADER_T *) out_buffer->_data;
        }

        memcpy(out_buffer->_data + out_buffer->_size, pkt.data, pkt.size);
        out_buffer->_size += pkt.size;
//        rfbLog("Write packet %3"PRId64" (size=%5d), frame=%"PRId64"\n", pkt.pts, pkt.size, frame->pts);
        av_packet_unref(&pkt);
    }

    header->HEADER.ffmpeg_body_len = Swap32IfLE(out_buffer->_size - FFMPEG_HEADER_LEN);
//    rfbErr("Body size: %lu\n", out_buffer->_size);

    return TRUE;
}
