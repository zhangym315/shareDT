#include "ExportImages.h"

int mainExport(const char ** cmdArg, const struct cmdConf * conf)
{
    ExportImages ei(conf->argc, const_cast<char **>(conf->argv));
    return 0;
}

ExportImages::ExportImages(int argc, char ** argv)
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
            _option.frequency = stoi(*(++i));
            continue;
        } else if ( (*i) == "--format" ) {
            if( *(++i) == "RGB" )
                _option.format = ExportImagesOptions::EXPORT_RGB;
            else if ( *i == "YUV" )
                _option.format = ExportImagesOptions::EXPORT_YUV;
            continue;
        }
    }
}
