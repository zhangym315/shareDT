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
                     _row_pointers(nullptr), _X(0), _Y(0),
                     _fb(nullptr) { }
    int init(int argc, char ** argv);
    ~ExportImages() { }

    int startExportImages();

  private:
    int parseExportImagesOptions();
    void writeToFile(const String & file);
    void checkBufferSize();

    Format       _format;
    unsigned int _total;
    png_bytep  * _row_pointers;
    size_t       _X;
    size_t       _Y;
    FrameBuffer * _fb;
};

#endif //_EXPORTIMAGES_H_
