#include <png.h>

#include "gtest/gtest.h"
#include "FfmpegEncodeDecode.h"

extern "C" {
#include <libswscale/swscale.h>
}

TEST(ffmpeg_encode_decode, ffmpeg_encode) {
    FfmpegEncodeDecodeFrameTesting coder(1024, 2048, 50, AV_PIX_FMT_RGB24);

    EXPECT_TRUE(coder.valid());
    EXPECT_TRUE(coder.getSampleFrame(10));
    coder.toRGBandExportToFiles(coder.getSrcFrames(), "Input_RDB32_");

    EXPECT_TRUE(coder.encodeToBuffer());


    coder.decodeToFrame();
    coder.toRGBandExportToFiles(coder.getDstFrames(), "Output_RDB32_");
}

TEST(ffmpeg_encode_decode, ffmpeg_decode) {
    //decode(NULL, NULL, NULL);
}

FfmpegEncodeDecodeFrameTesting::FfmpegEncodeDecodeFrameTesting(int w, int h, int totalFrames,
                                                   AVPixelFormat pixFormat, const std::string & codecName ) :
                                                    _width(w),
                                                    _height(h),
                                                    _totalFrames(totalFrames),
                                                    _valid(false),
                                                    _pixFormat(pixFormat),
                                                    _pts(0),
                                                    _parser(NULL),
                                                    _packet(NULL),
                                                    _decCtx(NULL),
                                                    _decodec(NULL),
                                                    _sws_yuv420_to_rgbx32(NULL),
                                                    _isLastFrameUnused(false)
{
    _codec = avcodec_find_encoder_by_name(codecName.c_str());
    if (!_codec)
        return;

    _encCtx = avcodec_alloc_context3(_codec);
    if (!_encCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return;
    }

    /* put sample parameters */
    _encCtx->bit_rate = 400000;
    /* resolution must be a multiple of two */
    _encCtx->width = _width;
    _encCtx->height = _height;
    /* frames per second */
    _encCtx->time_base = (AVRational){1, 25};
    _encCtx->framerate = (AVRational){25, 1};

    /* emit one intra _originalFrame every ten frames
     * check _originalFrame pict_type before passing _originalFrame
     * to encoder, if _originalFrame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I _originalFrame irrespective to gop_size
     */
    _encCtx->gop_size = 10;
    _encCtx->max_b_frames = 1;
    _encCtx->pix_fmt = pixFormat;

    int ret = avcodec_open2(_encCtx, _codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        return;
    }

    // init decoder
    _decodec = avcodec_find_decoder_by_name(codecName.c_str());
    if (!_decodec) {
        fprintf(stderr, "Codec not found\n");
        return;
    }

    _decCtx = avcodec_alloc_context3(_decodec);
    if (!_decCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return;
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(_decCtx, _decodec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return;
    }

    _sws_yuv420_to_rgbx32 = sws_getContext(_width, _height, AV_PIX_FMT_YUV420P,
                                           _width, _height, AV_PIX_FMT_RGB32,
                                           SWS_BICUBIC, NULL,NULL,NULL);
    _valid = true;
}

FfmpegEncodeDecodeFrameTesting::~FfmpegEncodeDecodeFrameTesting()
{
//    avcodec_free_context(&_encCtx);
//    av_frame_free(&_originalFrame);
    for (auto & frame : _originalFrames) {
        av_frame_free(&frame);
    }

    for (auto & buf : _encodeBuffer) {
        if (buf._capacity > 0) {
            buf._capacity = 0;
            free(buf._data);
        }
    }
}

bool FfmpegEncodeDecodeFrameTesting::valid()
{
    return _valid;
}

bool FfmpegEncodeDecodeFrameTesting::getSampleFrame(int framesCnt)
{
    for (auto & frame : _originalFrames) {
        av_frame_free(&frame);
    }

    _totalFrames = framesCnt;
    while (framesCnt--) {

        AVFrame * frame = av_frame_alloc();
        if (!frame) {
            fprintf(stderr, "Could not allocate video frame\n");
            return false;
        }
        frame->format = _encCtx->pix_fmt;
        frame->width  = _encCtx->width;
        frame->height = _encCtx->height;

        if (av_frame_get_buffer(frame, 0) < 0) {
            fprintf(stderr, "Could not allocate the video frame data\n");
            return false;
        }

        if (_pixFormat == AV_PIX_FMT_YUV420P) {
            /* make sure the frame data is writable */
            if (av_frame_make_writable(frame) < 0)
                return false;

            /* prepare a dummy image */
            /* Y */
            for (int y = 0; y < _encCtx->height; y++) {
                for (int x = 0; x < _encCtx->width; x++) {
                    frame->data[0][y * frame->linesize[0] + x] = x + y + (std::rand() % 30) * 3;
                }
            }

            /* Cb and Cr */
            for (int y = 0; y < _encCtx->height/2; y++) {
                for (int x = 0; x < _encCtx->width/2; x++) {
                    frame->data[1][y * frame->linesize[1] + x] = 128 + y + (std::rand() % 30) * 2;
                    frame->data[2][y * frame->linesize[2] + x] = 64 + x + (std::rand() % 30) * 5;
                }
            }

            frame->pts = _pts++;
        } else if (_pixFormat == AV_PIX_FMT_RGB24) {
            /* make sure the frame data is writable */
            if (av_frame_make_writable(frame) < 0)
                return false;

            for (int y = 0; y < _encCtx->height; y++) {
                int lineStartsBytes = y*_encCtx->width*3;
                for (int x = 0; x < _encCtx->width; x++) {
                    frame->data[0][lineStartsBytes + x*3 + 0] = y/128;
                    frame->data[0][lineStartsBytes + x*3 + 1] = y/128;
                    frame->data[0][lineStartsBytes + x*3 + 2] = y/128;
                }
            }
            frame->pts = _pts++;
        }

        _originalFrames.emplace_back(frame);
    }
    return true;
}

bool FfmpegEncodeDecodeFrameTesting::encodeToBuffer()
{
    for(int i=0; i<_originalFrames.size() ; i++) {
        AVPacketBuf buffer = { 0 };
        AVPacketBuf bufSrc = { 0 };
        if (!encode(_encCtx, _originalFrames[i], &bufSrc)) return false;

        if (!realloc_total_packet_buf(&buffer, bufSrc._size))
            return false;

        memcpy(buffer._data, bufSrc._data, bufSrc._size);
        buffer._size = bufSrc._size;
        _encodeBuffer.emplace_back(buffer);
    }

    return true;
}

const std::vector<AVPacketBuf> & FfmpegEncodeDecodeFrameTesting::getBuffer()
{
    return _encodeBuffer;
}

AVFrame * FfmpegEncodeDecodeFrameTesting::allocateFrame()
{
    AVFrame * frame = av_frame_alloc();

    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return NULL;
    }
    frame->format = _encCtx->pix_fmt;
    frame->width  = _encCtx->width;
    frame->height = _encCtx->height;

    if (av_frame_get_buffer(frame, 0) < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        return NULL;
    }

    return frame;
}

bool FfmpegEncodeDecodeFrameTesting::decodeToFrame()
{
    if (_parser == NULL && ( _parser = av_parser_init(_codec->id)) == NULL)
    {
        rfbErr("parser not found\n");
        return FALSE;
    }

    if (_packet == NULL && (_packet=av_packet_alloc()) == NULL) {
        rfbErr("Failed to allocate packet for decoding\n");
        return FALSE;
    }

    for (int i=0; i<_encodeBuffer.size(); i++)
    {


        if (av_parser_parse2(_parser, _decCtx, &_packet->data, &_packet->size,
                             _encodeBuffer[i]._data+16, _encodeBuffer[i]._size-16,
                             AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0) < 0) {
            fprintf(stderr, "Error while parsing\n");
            return false;
        }

        if (_packet->size) {
            AVFrame * frame = (_isLastFrameUnused && !_dstFrames.empty()) ? _dstFrames.back() : allocateFrame();
            _dstFrames.emplace_back(frame);
            if (decode(_decCtx, frame, _packet)) {
                std::cout << "dstFrame Size0: " << frame->linesize[0] << " size1: " << frame->linesize[1] << std::endl;
                frame = allocateFrame();
                _dstFrames.emplace_back(frame);

                while (fetch_frame(_decCtx, frame)) {
                    frame = allocateFrame();
                    _dstFrames.emplace_back(frame);
                }
            }
            _isLastFrameUnused = true;
        }

    }

    return true;
}

void FfmpegEncodeDecodeFrameTesting::toRGBandExportToFiles(const std::vector<AVFrame *> & frameVector, const std::string & prefix)
{
    int i = 0;
    unsigned char * buffer = (unsigned char* ) malloc(_width * _height * 4);
    for (const auto & frame : frameVector) {
        int data[8] = {(int) _width * 3, 0, 0, 0, 0, 0, 0, 0};
        uint8_t * srcSlice[8] = { (uint8_t *) buffer, 0, 0, 0, 0, 0, 0, 0 };

        sws_scale ( _sws_yuv420_to_rgbx32, (const uint8_t *const *)(frame->data), (const int *)(frame->linesize), 0,
                    _height, srcSlice, data );
        exportRGBX32ToFiles(prefix + std::to_string(i++) + ".png", buffer, _width, _height);
    }
}

void FfmpegEncodeDecodeFrameTesting::exportRGBX32ToFiles(const std::string &path, unsigned char *buffer,
                                                         size_t w, size_t h)
{
    write_RGB32_image(path.c_str(), buffer, w, h);
}
