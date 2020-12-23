#ifndef _EXPORTIMAGES_H_
#define _EXPORTIMAGES_H_

#include "MainConsole.h"
#include "StartServer.h"

extern int mainExport(const char ** cmdArg, const struct cmdConf * conf);

struct ExportImagesOptions {
    ExportImagesOptions() : frequency(10), format(EXPORT_INVALID) { }

    unsigned int frequency;
    enum Format { EXPORT_RGB, EXPORT_YUV, EXPORT_INVALID};
    Format       format;
};

class ExportImages final : public StartCapture {
  public:
    ExportImages(int argc, char ** argv);

  private:
    void parseExportImagesOptions();
    ExportImagesOptions _option;
};

#endif //_EXPORTIMAGES_H_
