#include "ExportImages.h"

int mainExport(const char ** cmdArg, const struct cmdConf * conf)
{
    ExportImages ei;
    char ** argv = const_cast<char **>(conf->argv);
    if ( ei.init(conf->argc, argv) != RETURN_CODE_SUCCESS)
    {
        std::cout << "Failed to init ExportImages for: " << ei.getWID();
        return RETURN_CODE_INTERNAL_ERROR;
    }

    std::cout << "Starting to export images for: " << ei.getWID() << std::endl;

    return ei.startExportImages();
}

int ExportImages::init(int argc, char ** argv)
{
    if(StartCapture::init(argc,  argv) || parseExportImagesOptions())
        return RETURN_CODE_INVALID_ARG;
    return RETURN_CODE_SUCCESS;
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
        } else if ( (*i) == "--total" ) {
            _total = stoi(*(++i));
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

    if ( _format == ExportImages::EXPORT_RGB ) {
    }
    else if ( _format == ExportImages::EXPORT_RGB ) {
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

    return RETURN_CODE_SUCCESS;
}

void ExportImages::writeToFile(const String & file)
{
    int y;
    unsigned int width = _sp->getWidth();
    unsigned int height = _sp->getHeight();
    FILE *fp = fopen(file.c_str(), "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(
            png,
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

    png_write_end(png, NULL);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

}