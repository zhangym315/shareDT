#include "CaptureInfo.h"

CaptureInfo* CaptureInfo::_instance = nullptr;

CaptureInfo::CaptureInfo() { }

CaptureInfo * CaptureInfo::instance()
{
    if (_instance == nullptr)
    {
        _instance = new CaptureInfo();
    }
    return _instance;
}
