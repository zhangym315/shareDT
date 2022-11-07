#ifndef SHAREDT_FFMPEGENCODEDECODE_H
#define SHAREDT_FFMPEGENCODEDECODE_H

extern "C" {
#include "ffmpeg_interface.h"
#include "ffmpeg_server_interface.h"
#include "ffmpeg_client_interface.h"
}

#include "TypeDef.h"

#ifdef av_err2str
#undef av_err2str
#include <string>
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif  // av_err2str

class FfmpegEncodeDecodeFrameTesting{
public:
    FfmpegEncodeDecodeFrameTesting() : _encCtx(NULL), _codec(NULL),
                                 _pixFormat(AV_PIX_FMT_NONE), _width(0),
                                 _height(0), _totalFrames(0), _pts(0),
                                 _parser(NULL), _packet(NULL), _sws_yuv420_to_rgbx32(NULL),
                                 _decCtx(NULL), _decodec(NULL) {
        _originalFrames.clear();
        _encodeBuffer.clear();
    }
    FfmpegEncodeDecodeFrameTesting(int w, int h, int totalFrames = 1, AVPixelFormat pixFormat = AV_PIX_FMT_YUV420P,
                                     const std::string & codecName = "libx265");
    ~FfmpegEncodeDecodeFrameTesting();

    bool getSampleFrame(int framesCnt=0);
    bool valid();
    bool encodeToBuffer();
    bool decodeToFrame();
    AVFrame * allocateFrame();
    void toRGBandExportToFiles(const std::vector<AVFrame *> & frameVector, const std::string & prefix);
    void exportRGBX32ToFiles(const std::string & path, unsigned char * buffer, size_t w, size_t h);

    [[nodiscard]] const std::vector<AVFrame *> & getDstFrames() const { return _dstFrames; }
    [[nodiscard]] const std::vector<AVFrame *> & getSrcFrames() const { return _originalFrames; }
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
    struct SwsContext * _sws_yuv420_to_rgbx32;
    bool _isLastFrameUnused;  /* true for unused last frame in _dstFrames */

};

#endif //SHAREDT_FFMPEGENCODEDECODE_H
