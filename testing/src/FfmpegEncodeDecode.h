#ifndef SHAREDT_FFMPEGENCODEDECODE_H
#define SHAREDT_FFMPEGENCODEDECODE_H

extern "C" {
#include "ffmpeg_interface.h"
#include "ffmpeg_server_interface.h"
#include "ffmpeg_client_interface.h"
}

#include "TypeDef.h"

class FfmpegEncodeDecodeFrameTesting{
public:
    FfmpegEncodeDecodeFrameTesting() : _encCtx(NULL), _codec(NULL),
                                 _pixFormat(AV_PIX_FMT_NONE), _width(0),
                                 _height(0), _totalFrames(0), _pts(0),
                                 _parser(NULL), _packet(NULL), _decCtx(NULL),
                                 _decodec(NULL) {
        _originalFrames.clear();
        _encodeBuffer.clear();
    }
    FfmpegEncodeDecodeFrameTesting(int w, int h, int totalFrames = 1, AVPixelFormat pixFormat = AV_PIX_FMT_YUV420P,
                             const String & codecName = codec_name);
    ~FfmpegEncodeDecodeFrameTesting();

    bool getSampleFrame(int framesCnt=0);
    bool valid();
    bool encodeToBuffer();
    bool decodeToFrame();
    AVFrame * allocateFrame();

    const std::vector<AVPacketBuf> & getBuffer();
private:
    AVCodecContext * _encCtx;

    /* source encoding */
    std::vector<AVFrame *> _originalFrames;
    std::vector<AVPacketBuf> _encodeBuffer;
    const AVCodec * _codec;
    AVPixelFormat _pixFormat;
    int  _width, _height;
    int  _totalFrames;
    bool _valid;
    int64_t  _pts;

    /* decoding */
    const AVCodec * _decodec;
    AVCodecContext * _decCtx;
    std::vector<AVFrame *> _dstFrames;
    AVCodecParserContext * _parser;
    AVPacket * _packet;
    bool _isLastFrameUnused;  /* true for unused last frame in _dstFrames */

};

#endif //SHAREDT_FFMPEGENCODEDECODE_H
