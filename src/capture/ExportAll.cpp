#include <unordered_set>
#include "ExportAll.h"
#include <png.h>
#include "SamplesProvider.h"

FrameBuffer * ExportAll::getFrameBuffer(CircWRBuf<FrameBuffer> & cwf)
{
    FrameBuffer * fb;
    cwf.setEmpty();
#ifdef __SHAREDT_IOS__
    // IOS monitor capture is different
    if (_type == SP_MONITOR) {
        const CapMonitor & cp = CapMonitor::getById(_captureId);
        if (!cp.isValid()) return nullptr;

        FrameProcessorWrap * fpw = FrameProcessorWrap::instance();
        fpw->setReInitiated();

        fpw->setMV(const_cast<CapMonitor *>(&cp), 1);
        fpw->setCFB(&cwf);

        int indicator = 0;
        while(!fpw->isReady() && ++indicator < 100) {
            std::this_thread::sleep_for(50ms);
        }
        if (!fpw->isReady()) return nullptr;

        fpw->resume();
        while ((fb = cwf.getToRead()) == nullptr && ++indicator < 200) {
            std::this_thread::sleep_for(50ms);
        }

        if (fb) fb->setWidthHeight(cp.getOrgWidth(), cp.getOrgHeight());

        return fb;
    }
#endif

    fb = cwf.getToWrite();
    return FrameGetter::exportAllFrameGetter(fb, _type, _captureId) ? fb : nullptr;
}


#ifdef __SHAREDT_IOS__
const static std::unordered_set<String> PROCESS_FILTER{"Menubar", "Fullscreen Backdrop", "Desktop", "Dock"};
const static std::unordered_set<String> PROCESS_FILTER_CONTAINS {"Desktop Picture"};
#elif __SHAREDT_WIN__
const static std::unordered_set<String> PROCESS_FILTER {"default ime", "msctfime ui", "radeonsettings"};
const static std::unordered_set<String> PROCESS_FILTER_CONTAINS {};
#else
const static std::unordered_set<String> PROCESS_FILTER {};
const static std::unordered_set<String> PROCESS_FILTER_CONTAINS {};
#endif

bool ExportAll::filterExportWinName(const String & w)
{
    if (PROCESS_FILTER.find(w) != PROCESS_FILTER.end()) return true;
    auto it = find_if(PROCESS_FILTER_CONTAINS.begin(), PROCESS_FILTER_CONTAINS.end(),
                      [&](const String & name) -> bool {
                          return w.find(name) != std::string::npos;
                      });

    return it != PROCESS_FILTER_CONTAINS.end();
}

void ExportAll::writeToFile(const String &p, const FrameBuffer *f)
{
    unsigned int width = f->getWidth();
    unsigned int height = f->getHeight();
    FILE *fp = fopen(p.c_str(), "wb");
    if(!fp) return ;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(png,
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
        png_write_row(png, (png_bytep)(f->getData() + i*width*4));
    }

    png_write_end(png, nullptr);
    fclose(fp);

    png_destroy_write_struct(&png, &info);

}

