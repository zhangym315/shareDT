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
    FrameBuffer * fb;

    _sp->sampleResume();

    std::chrono::microseconds duration(MICROSECONDS_PER_SECOND/_frequency);
    for ( int i=0 ; i<_total ; )
    {
        if ( (fb = _sp->getFrameBuffer()) == nullptr ) {
            _sp->sampleResume();
            std::this_thread::sleep_for(duration);
            continue;
        }

        i++;
        std::cout << "SampleProvider returns data: " << i << std::endl;
    }

    return RETURN_CODE_SUCCESS;
}
