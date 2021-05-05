#include "ExportImages.h"
#include "x265.h"

extern "C" {
#include "ReadWriteVideo.h"
#include "libswscale/swscale.h"
}

int mainExport(const char ** cmdArg, const struct cmdConf * conf)
{
    ExportImages ei;
    char ** argv = const_cast<char **>(conf->argv);
    int ret;
    if ( (ret = ei.initOptions(conf->argc, argv)) != RETURN_CODE_SUCCESS ||
         (ret = ei.initSrceenProvider()) != RETURN_CODE_SUCCESS )
    {
        if(ret == RETURN_CODE_INVALID_ARG) {
            ei.exportUsage();
            return ret;
        } else {
            std::cout << "Failed to init ExportImages for: " << ei.getWID() << std::endl;
            return RETURN_CODE_INTERNAL_ERROR;
        }
    }

    std::cout << "Starting to export images for: " << ei.getWID() << std::endl;

    return ei.start();
}

int ExportImages::start ()
{
    if(_mp4) {
        return startExportH265Video();
    } else {
        return startExportImages();
    }
}

int ExportImages::initOptions(int argc, char ** argv)
{
    int ret;
    if((ret = StartCapture::initParsing(argc,  argv)) != RETURN_CODE_SUCCESS ||
        (ret = parseExportImagesOptions()) != RETURN_CODE_SUCCESS )
    {
        return ret;
    }
    return RETURN_CODE_SUCCESS;
}

void ExportImages::exportUsage()
{
    std::cerr << "\n";
    std::cerr << "The following options related with export command option"  << std::endl;
    std::cerr << "--format                     Format of exported image, supported RGB and YUV" << std::endl;
    std::cerr << "--total                      Total number of images that export command can capture" << std::endl;
    std::cerr << "--mp4                        Export images as mp4 video" << std::endl;
    std::cerr << "\n";
    std::cerr << "\n";
}

int ExportImages::parseExportImagesOptions()
{
    const StringVec & options = getUnrecognizedOptions();
    for (auto i = options.begin(); i != options.end(); ++i) {
        if ( (*i) == "--format" ) {
            if( *(++i) == "RGB" || *i == "rgb" )
                _format = ExportImages::EXPORT_RGB;
            else if ( *i == "YUV" || *i == "yuv" )
                _format = ExportImages::EXPORT_YUV;
            else
                return RETURN_CODE_INVALID_ARG;
        } else if ( (*i) == "--total" ) {
            _total = stoi(*(++i));
        } else if ( (*i) == "--mp4" ) {
            FrameProcessorWrap::instance()->setImageTypeToRGB();
            _mp4 = true;
        } else {
            std::cerr << "Invalid options: " << (*i);
            return RETURN_CODE_INVALID_ARG;
        }
    }

    return RETURN_CODE_SUCCESS;
}

int ExportImages::startExportImages()
{
    if (_sp == nullptr){
        LOGGER.error() << "Failed to start server";
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!_sp->startSample()) {
        LOGGER.error() << "Failed to start SampleProvider" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    while ( !_sp->isSampleReady() ) {
        std::this_thread::sleep_for(50ms);
    }
    _sp->sampleResume();

    std::chrono::microseconds duration(MICROSECONDS_PER_SECOND/_frequency);
    for ( int i=0 ; i<_total ; )
    {
        auto start = std::chrono::system_clock::now();
        if ( (_fb = _sp->getFrameBuffer()) == nullptr || (_fb->getData() == nullptr)) {
            _sp->sampleResume();
            std::this_thread::sleep_for(duration);
            continue;
        }
        std::cout << "Getting data for : " << i << ", gettingTime=" <<
                  (std::chrono::system_clock::now()-start).count()/1000 << "ms" << std::endl;

        start = std::chrono::system_clock::now();
        writeToFile(getCapServerPath() + PATH_SEP_STR + "EXPORTED_" + std::to_string(i) + ".png");
        std::cout << "SampleProvider returns data: " << i << ", writtingTime=" <<
                   (std::chrono::system_clock::now()-start).count()/1000 << "ms" << std::endl;

        i++;
    }

    std::cout << "Images are exported to " << getCapServerPath();
    return RETURN_CODE_SUCCESS;
}

int ExportImages::startExportH265Video()
{
    ffmpeg_audio_video_input ffmpegInput;
    ffmpeg_video_frame       ffmpegFrame;

    if (_sp == nullptr) {
        LOGGER.error() << "Failed to start server";
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!_sp->startSample()) {
        LOGGER.error() << "Failed to start SampleProvider" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    String outfile = getCapServerPath() + PATH_SEP_STR + "EXPORTED_OUTFILE.mp4";

    while ( !_sp->isSampleReady() ) {
        std::this_thread::sleep_for(50ms);
    }
    _sp->sampleResume();

    ffmpegInput.rate = _frequency;
    ffmpegInput.w = _sp->getWidth();
    ffmpegInput.h = _sp->getHeight();
    ffmpegInput.infra_frame = 1;
    ffmpegFrame.rgb_to_yuv_ctx = sws_getContext(_sp->getWidth(), _sp->getHeight(),
                                                AV_PIX_FMT_RGB24,
                                                _sp->getWidth(), _sp->getHeight(),
                                                AV_PIX_FMT_YUV420P,
                                                SWS_BICUBIC, NULL,NULL,NULL);
    ffmpegFrame.total_time = _total / _frequency;
    export_video_open(&ffmpegInput, outfile.c_str());

    auto start = std::chrono::system_clock::now();
    std::chrono::microseconds duration(MICROSECONDS_PER_SECOND/_frequency);
    for ( int i=0 ; i<_total ; )
    {
        if ( (_fb = _sp->getFrameBuffer()) == nullptr || (_fb->getData() == nullptr)) {
            _sp->sampleResume();
            std::this_thread::sleep_for(duration);
            continue;
        }

        ffmpegFrame.w = _sp->getWidth();
        ffmpegFrame.h = _sp->getHeight();
        ffmpegFrame.frame_index = i;
        ffmpegFrame.format = AV_PIX_FMT_RGB24;
        ffmpegFrame.data0 = _fb->getData();
        ffmpegFrame.data0_len = _fb->getPacity();

        std::cout << "Getting data for : " << i << ", gettingTime=" <<
                  (std::chrono::system_clock::now()-start).count()/1000 << "ms" <<
                  " size: " << _fb->getSize() << std::endl;

        export_video_write(&ffmpegFrame);
        i++;
    }

    export_video_close();

    cout << "Video file write to: " << outfile;
    return RETURN_CODE_SUCCESS;
}

void ExportImages::writeToFile(const String & file)
{
    unsigned int width = _sp->getWidth();
    unsigned int height = _sp->getHeight();
    FILE *fp = fopen(file.c_str(), "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(png,
                 info,
                 width, height,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT
                );
    png_write_info(png, info);

    for ( int i=0 ; i<height ; i++) {
        png_write_row(png, (png_bytep)(_fb->getData() + i*width*4));
    }

    png_write_end(png, nullptr);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

}

int ExportImages::_startExportH265Video(const String & infile,
                                        int width,
                                        int height,
                                        int type,
                                        const String & outfile)
{
    FILE *fp_src = nullptr;
    FILE *fp_dst = nullptr;
    unsigned int luma_size = 0;
    unsigned int chroma_size = 0;
    //int buff_size = 0;
    char *buff = nullptr;
    uint32_t i_nal = 0;
    int i_frame = 0;
    int ret = 0;

    x265_param param;
    x265_nal *nal = nullptr;
    x265_encoder *handle = nullptr;
    x265_picture *pic_in = nullptr;

    int csp = type; // X265_CSP_I420;

    fp_src = fopen (infile.c_str(), "rb");
    fp_dst = fopen (outfile.c_str(), "wb");
    if (fp_src == nullptr || fp_dst == nullptr)
    {
        perror ("Error open yuv files:");
        return -1;
    }

    x265_param_default (&param);
    param.bRepeatHeaders = 1;//write sps,pps before keyframe
    param.internalCsp = csp;
    param.sourceWidth = width;
    param.sourceHeight = height;
    param.fpsNum = 25;
    param.fpsDenom = 1;

    handle = x265_encoder_open (&param);
    if (handle == nullptr)
    {
        printf ("x265_encoder_open err\n");
        goto out;
    }

    pic_in = x265_picture_alloc ();
    if (pic_in == nullptr)
    {
        goto out;
    }
    x265_picture_init (&param, pic_in);

    luma_size = param.sourceWidth * param.sourceHeight;
    switch (csp)
    {
        case X265_CSP_I444:buff = (char *) malloc (luma_size * 3);
            pic_in->planes[0] = buff;
            pic_in->planes[1] = buff + luma_size;
            pic_in->planes[2] = buff + luma_size * 2;
            pic_in->stride[0] = width;
            pic_in->stride[1] = width;
            pic_in->stride[2] = width;
            break;
        case X265_CSP_I420:buff = (char *) malloc (luma_size * 3 / 2);
            pic_in->planes[0] = buff;
            pic_in->planes[1] = buff + luma_size;
            pic_in->planes[2] = buff + luma_size * 5 / 4;
            pic_in->stride[0] = width;
            pic_in->stride[1] = width / 2;
            pic_in->stride[2] = width / 2;
            break;
        case X265_CSP_I422:
            buff = (char *) malloc (luma_size * 3 / 2);
            pic_in->planes[0] = buff;
            pic_in->planes[1] = buff + luma_size;
            pic_in->planes[2] = buff + luma_size * 5 / 4;
            pic_in->stride[0] = width;
            pic_in->stride[1] = width / 2;
            pic_in->stride[2] = width / 2;
        default:
        printf ("Colorspace Not Support.\n");
            goto out;
    }

    fseek (fp_src, 0, SEEK_END);
    switch (csp)
    {
        case X265_CSP_I444:i_frame = ftell (fp_src) / (luma_size * 3);
            chroma_size = luma_size;
            break;
        case X265_CSP_I420:i_frame = ftell (fp_src) / (luma_size * 3 / 2);
            chroma_size = luma_size / 4;
        case X265_CSP_I422:i_frame = ftell (fp_src) / (luma_size * 3 / 2);
            chroma_size = luma_size / 4;
            break;
        default:printf ("Colorspace Not Support.\n");
            return -1;
    }
    fseek (fp_src, 0, SEEK_SET);
    printf ("framecnt: %d, y: %d u: %d\n", i_frame, luma_size, chroma_size);

    for (int i = 0; i < i_frame; i++)
    {
        switch (csp)
        {
            case X265_CSP_I444:
            case X265_CSP_I420:
            case X265_CSP_I422:
            if (fread (pic_in->planes[0], 1, luma_size, fp_src) != luma_size)
                break;
            if (fread (pic_in->planes[1], 1, chroma_size, fp_src) != chroma_size)
                break;
            if (fread (pic_in->planes[2], 1, chroma_size, fp_src) != chroma_size)
                break;
            break;
            default:printf ("Colorspace Not Support.\n");
                goto out;
        }

        ret = x265_encoder_encode (handle, &nal, &i_nal, pic_in, nullptr);
        printf ("encode frame: %5d framesize: %d nal: %d\n", i + 1, ret, i_nal);
        if (ret < 0)
        {
            printf ("Error encode frame: %d.\n", i + 1);
            goto out;
        }

        for (uint32_t j = 0; j < i_nal; j++)
        {
            fwrite (nal[j].payload, 1, nal[j].sizeBytes, fp_dst);
        }
    }
    // Flush Decoder
    while ((ret = x265_encoder_encode (handle, &nal, &i_nal, nullptr, nullptr)))
    {
        static int cnt = 1;
        printf ("flush frame: %d\n", cnt++);
        for (uint32_t j = 0; j < i_nal; j++)
        {
            fwrite (nal[j].payload, 1, nal[j].sizeBytes, fp_dst);
        }
    }

    out:
    x265_encoder_close (handle);
    x265_picture_free (pic_in);
    if (buff)
        free (buff);

    fclose (fp_src);
    fclose (fp_dst);

    return 0;
}
