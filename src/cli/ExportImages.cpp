#include "ExportImages.h"

int mainExport(const char ** cmdArg, const struct cmdConf * conf)
{
    ExportImages ei(conf->argc, const_cast<char **>(conf->argv));

    char ** argv = const_cast<char **>(conf->argv);
    if ( ei.init(conf->argc, argv) != RETURN_CODE_SUCCESS)
    {
        std::cout << "Failed to init ExportImages for: " << ei.getWID();
        return RETURN_CODE_INTERNAL_ERROR;
    }

    std::cout << "Starting to export images for: " << ei.getWID();

    return ei.startExportImages();
}

ExportImages::ExportImages(int argc, char ** argv) : ExportImages()
{
    if(initParsing(argc,  argv))
        return;
    parseExportImagesOptions();
}

void ExportImages::parseExportImagesOptions()
{
    const StringVec & options = getUnrecognizedOptions();
    for (auto i = options.begin(); i != options.end(); ++i) {
        if ( (*i) == "--frequency" ) {
            _frequency = stoi(*(++i));
            continue;
        } else if ( (*i) == "--format" ) {
            if( *(++i) == "RGB" || *i == "rgb" )
                _format = ExportImages::EXPORT_RGB;
            else if ( *i == "YUV" || *i == "yuv" )
                _format = ExportImages::EXPORT_YUV;
            continue;
        } else if ( (*i) == "--total" ) {
            _total = stoi(*(++i));
            continue;
        }
    }
}

int ExportImages::startExportImages()
{
    ScreenProvider *sp = getScreenProvide();
    if (sp == nullptr){
        LOGGER.error() << "Failed to start server";
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if(!sp->startSample()) {
        LOGGER.error() << "Failed to start SampleProvider" ;
        return RETURN_CODE_INTERNAL_ERROR;
    }

    if ( _format == ExportImages::EXPORT_RGB ) {
    }
    else if ( _format == ExportImages::EXPORT_RGB ) {
    }

    while ( !sp->isSampleReady() ) {
        std::this_thread::sleep_for(50ms);
    }
    FrameBuffer * fb;

    for ( int i=0 ; i<_total ; i++)
    {
        if ( (fb = sp->getFrameBuffer()) == nullptr )
            continue;

    }

    return RETURN_CODE_SUCCESS;
}
