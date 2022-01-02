#ifndef SHAREDT_EXPORTALL_H
#define SHAREDT_EXPORTALL_H
#include "Buffer.h"
#include "WindowProcessor.h"

class ExportAll {
public:
    ExportAll(SPType type, int id) : _type(type), _captureId(id) { }
    ~ExportAll() { }

    FrameBuffer * getFrameBuffer(CircWRBuf<FrameBuffer> & cwf);

private:
    SPType _type;
    int    _captureId;
};

#endif //SHAREDT_EXPORTALL_H
