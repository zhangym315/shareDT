#ifndef _EXPORTIMAGES_H_
#define _EXPORTIMAGES_H_

#include <png.h>
#include "MainConsole.h"
#include "StartServer.h"

extern int mainExport(const char ** cmdArg, const struct cmdConf * conf);

class ExportImages final : public StartCapture {
    enum Format { EXPORT_RGB, EXPORT_YUV, EXPORT_INVALID};
  public:
    ExportImages() : _format(EXPORT_INVALID), _total(100),
                     _row_pointers(nullptr),
                     _fb(nullptr), _mp4(false) { }
    int initOptions(int argc, char ** argv);
    ~ExportImages() = default;

    int startExportImages();
    int startExportH265Video();
    int start();
    void exportUsage();

  private:
    int parseExportImagesOptions();
    void writeToFile(const String & file);
    int _startExportH265Video(const String & infile, int width, int height, int type, const String & outfile);

    Format       _format;
    unsigned int _total;
    bool         _mp4;
    png_bytep  * _row_pointers;
    FrameBuffer * _fb;
};

#endif //_EXPORTIMAGES_H_
