#ifndef _EXPORTIMAGES_H_
#define _EXPORTIMAGES_H_

#include <png.h>
#include "main/ShareDT.h"
#include "Capture.h"

extern int mainExport(const struct cmdConf * conf);

class ExportImages final : public Capture {
    enum Format { EXPORT_RGB, EXPORT_YUV, EXPORT_INVALID};
  public:
    enum EXPORTACTION { EXPORT_ALL, EXPORT_IMAGES, EXPORT_MP4 };
    ExportImages() : _format(EXPORT_INVALID), _total(100),
                     _row_pointers(nullptr),
                     _fb(nullptr), _action(EXPORT_IMAGES) { }
    int initOptions(int argc, char ** argv);
    ~ExportImages() = default;

    int startExportAll();
    int startExportImages();
    int startExportH265Video();
    int start();

    void exportUsage();

    [[nodiscard]] EXPORTACTION action() const { return _action; }

  private:
    int parseExportImagesOptions(int argc, char ** argv);
    int _startExportH265Video(const std::string & infile, int width, int height, int type, const std::string & outfile);

    Format       _format;
    unsigned int _total;
    EXPORTACTION _action;
    png_bytep  * _row_pointers;
    FrameBuffer *_fb;
};

#endif //_EXPORTIMAGES_H_
