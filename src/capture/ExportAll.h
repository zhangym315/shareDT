#ifndef SHAREDT_EXPORTALL_H
#define SHAREDT_EXPORTALL_H
#include "Buffer.h"
#include "WindowProcessor.h"

class ExportAll {
public:
    ExportAll(SPType type, int id) : _type(type), _captureId(id) { }
    ~ExportAll() { }

    FrameBuffer * getFrameBuffer(CircleWRBuf<FrameBuffer> & cwf);

    static void writeToFile(const std::string & p, const FrameBuffer * f);
    static bool filterExportWinName(const std::string & w);
private:
    SPType _type;
    int    _captureId;
};

#endif //SHAREDT_EXPORTALL_H
