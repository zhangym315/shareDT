#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
#include <libavutil/timecode.h>
#include <libavutil/bprint.h>

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;

static int width, height;
static enum AVPixelFormat pix_fmt;

static const char *src_filename = NULL;
static int video_stream_idx = -1, audio_stream_idx = -1;

static AVFrame *frame = NULL;
static AVPacket *pkt = NULL;

static int video_frame_count = 0;
static int audio_frame_count = 0;
static char * output_prefix   = "output_";

#include <stdio.h>
#include <string.h>

int ffmpeg_save_frame_as_jpeg(AVFrame *pFrame, const char * filename) {
    const AVCodec *jpegCodec = avcodec_find_encoder(AV_CODEC_ID_JPEG2000);
    if (!jpegCodec) {
            return -1;
        }

    AVCodecContext *jpegContext = avcodec_alloc_context3(jpegCodec);
    if (!jpegContext) {
            return -2;
        }

    jpegContext->pix_fmt = pFrame->format;
    jpegContext->height = pFrame->height;
    jpegContext->width = pFrame->width;
    jpegContext->time_base = (AVRational){1, 1};

    if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
            return -3;
        }

    FILE *JPEGFile;
    JPEGFile = fopen(filename, "wb");

    if (avcodec_send_frame(jpegContext, pFrame) < 0) {
            return -4;
        }

    int ret = 0;
    while (ret >= 0) {
            AVPacket packet = {0};
            ret = avcodec_receive_packet(jpegContext, &packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                    fprintf(stderr, "Error during encoding\n");
                    exit(1);
                }

            fwrite(packet.data, 1, packet.size, JPEGFile);
            av_packet_unref(&packet);
        }

    fclose(JPEGFile);
    avcodec_close(jpegContext);

    return 0;
}

static int output_video_frame(AVFrame *frame)
{
    if (frame->width != width || frame->height != height ||
        frame->format != pix_fmt) {
            /* To handle this change, one could call av_image_alloc again and
             * decode the following frames into another rawvideo file. */
            fprintf(stderr, "Error: Width, height and pixel format have to be "
                            "constant in a rawvideo file, but the width, height or "
                            "pixel format of the input video changed:\n"
                            "old: width = %d, height = %d, format = %s\n"
                            "new: width = %d, height = %d, format = %s\n",
                    width, height, av_get_pix_fmt_name(pix_fmt),
                    frame->width, frame->height,
                    av_get_pix_fmt_name(frame->format));
            return -1;
        }

    if(video_frame_count % 100 == 0) {
            printf("Decoding video frame : %d --> %d ...\n",
                   video_frame_count, (video_frame_count/100 + 1)*100);
        }
    video_frame_count++;

    char filename[100];
    sprintf(filename, "%s%07d.jpg", output_prefix, video_frame_count);

    int ret = ffmpeg_save_frame_as_jpeg(frame, filename);
    if(ret < 0) {
            printf("ffmpeg_save_frame_as_jpeg ret: %d\n", ret);
        }

    return 0;
}

/*
 * TODO: Used for now
 */
static int output_audio_frame(AVFrame *frame)
{
/*    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
    printf("audio_frame n:%d nb_samples:%d pts:%s\n",
           audio_frame_count++, frame->nb_samples,
           av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));
*/
    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
//    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;
}

static int decode_packet(AVCodecContext *dec, const AVPacket *pkt)
{
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
            fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
            return ret;
        }

    // get all the available frames from the decoder
    while (ret >= 0) {
            ret = avcodec_receive_frame(dec, frame);
            if (ret < 0) {
                    // those two return values are special and mean there is no output
                    // frame available, but there were no errors during decoding
                    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                        return 0;

                    fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
                    return ret;
                }

            // write the frame data to output file
            if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
                ret = output_video_frame(frame);
            else if (dec->codec->type == AVMEDIA_TYPE_AUDIO)
                ret = output_audio_frame(frame);
            else
                fprintf(stderr, "Error getting codec_type=%s\n",
                        av_get_media_type_string(dec->codec->type));

            av_frame_unref(frame);
            if (ret < 0)
                return ret;
        }

    return 0;
}

static int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_ctx,
                              enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
            fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                    av_get_media_type_string(type), src_filename);
            return ret;
        } else {
            stream_index = ret;
            st = fmt_ctx->streams[stream_index];

            /* find decoder for the stream */
            dec = avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec) {
                    fprintf(stderr, "Failed to find %s codec\n",
                            av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }

            /* Allocate a codec context for the decoder */
            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx) {
                    fprintf(stderr, "Failed to allocate the %s codec context\n",
                            av_get_media_type_string(type));
                    return AVERROR(ENOMEM);
                }

            /* Copy codec parameters from input stream to output codec context */
            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
                    fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                            av_get_media_type_string(type));
                    return ret;
                }

            /* Init the decoders */
            if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
                    fprintf(stderr, "Failed to open %s codec\n",
                            av_get_media_type_string(type));
                    return ret;
                }
            *stream_idx = stream_index;
        }

    return 0;
}

static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
            struct sample_fmt_entry *entry = &sample_fmt_entries[i];
            if (sample_fmt == entry->sample_fmt) {
                    *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
                    return 0;
                }
        }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

int decode_video_images (char * src_filename, char * prefix)
{
    int ret = 0;

    if (prefix)
        output_prefix = prefix;
    /* open input file, and allocate format context */
    if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
            fprintf(stderr, "Could not open source file %s\n", src_filename);
            exit(1);
        }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
            fprintf(stderr, "Could not find stream information\n");
            exit(1);
        }

    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
            width = video_dec_ctx->width;
            height = video_dec_ctx->height;
            pix_fmt = video_dec_ctx->pix_fmt;
        }

    open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO);

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);

    if (!fmt_ctx->streams[audio_stream_idx] && !fmt_ctx->streams[video_stream_idx]) {
            fprintf(stderr, "Could not find audio and video stream in the input, aborting\n");
            ret = 1;
            goto end;
        }

    frame = av_frame_alloc();
    if (!frame) {
            fprintf(stderr, "Could not allocate frame\n");
            ret = AVERROR(ENOMEM);
            goto end;
        }

    pkt = av_packet_alloc();
    if (!pkt) {
            fprintf(stderr, "Could not allocate packet\n");
            ret = AVERROR(ENOMEM);
            goto end;
        }

    /* read frames from the file */
    while (av_read_frame(fmt_ctx, pkt) >= 0) {
            // check if the packet belongs to a stream we are interested in, otherwise
            // skip it
            if (pkt->stream_index == video_stream_idx)
                ret = decode_packet(video_dec_ctx, pkt);
            else if (pkt->stream_index == audio_stream_idx)
                ret = decode_packet(audio_dec_ctx, pkt);
            av_packet_unref(pkt);
            if (ret < 0)
                break;
        }

    /* flush the decoders */
    if (video_dec_ctx)
        decode_packet(video_dec_ctx, NULL);
    if (audio_dec_ctx)
        decode_packet(audio_dec_ctx, NULL);

    printf("Decoded succeeded, num_frames=%d\n", video_frame_count);

    end:
    avcodec_free_context(&video_dec_ctx);
    avcodec_free_context(&audio_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_packet_free(&pkt);
    av_frame_free(&frame);

    return ret < 0;
}
