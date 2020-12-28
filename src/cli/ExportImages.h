#ifndef _EXPORTIMAGES_H_
#define _EXPORTIMAGES_H_

#include "MainConsole.h"
#include "StartServer.h"

extern int mainExport(const char ** cmdArg, const struct cmdConf * conf);

class ExportImages final : public StartCapture {
    enum Format { EXPORT_RGB, EXPORT_YUV, EXPORT_INVALID};
  public:
    ExportImages() : _format(EXPORT_INVALID), _total(100) { }
    int init(int argc, char ** argv);
    ~ExportImages() { }

    int startExportImages();

  private:
    int parseExportImagesOptions();
    Format       _format;
    unsigned int _total;
};

#endif //_EXPORTIMAGES_H_
