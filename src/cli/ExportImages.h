#ifndef _EXPORTIMAGES_H_
#define _EXPORTIMAGES_H_

#include "MainConsole.h"
#include "StartServer.h"

extern int mainExport(const char ** cmdArg, const struct cmdConf * conf);

class ExportImages final : public StartCapture {
    enum Format { EXPORT_RGB, EXPORT_YUV, EXPORT_INVALID};
  public:
    ExportImages() : _frequency(10), _format(EXPORT_INVALID), _total(100) { }
    ExportImages(int argc, char ** argv);

    int startExportImages();

  private:
    void parseExportImagesOptions();
    unsigned int _frequency;
    Format       _format;
    unsigned int _total;
};

#endif //_EXPORTIMAGES_H_
