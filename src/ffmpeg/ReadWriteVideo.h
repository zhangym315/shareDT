#ifndef _READWRITEVIDEO_H_
#define _READWRITEVIDEO_H_
#include <libavformat/avformat.h>

typedef struct _ffmpeg_audio_video_input {
    /*
     * Common
     */
    unsigned int rate;
    unsigned int bit_rate;

    /*
     * Video
     */
    unsigned int w;
    unsigned int h;
    unsigned int infra_frame;

    /*
     * Audio
     */
} ffmpeg_audio_video_input;

typedef struct _ffmpeg_video_frame {
    enum AVPixelFormat format;
    unsigned int w;
    unsigned int h;
    size_t   frame_index;
    size_t   total_time;
    struct SwsContext * rgb_to_yuv_ctx;

    unsigned char * data0;
    unsigned char * data1;
    unsigned char * data2;
    size_t data0_len;
    size_t data1_len;
    size_t data2_len;
} ffmpeg_video_frame;

extern int export_video_open(ffmpeg_audio_video_input *input, const char *filename);
extern int export_video_write(ffmpeg_video_frame * frame);
extern int export_video_close();
#endif //_READWRITEVIDEO_H_
