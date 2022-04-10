#include "CaptureInfo.h"

CaptureInfo* CaptureInfo::_instance = nullptr;

CaptureInfo::CaptureInfo() : _monPtr(nullptr), _type(SPType::SP_NULL), _isServerRunning(false) { }

CaptureInfo * CaptureInfo::instance()
{
    if (_instance == nullptr)
    {
        _instance = new CaptureInfo();
    }
    return _instance;
}

void CaptureInfo::setIsRunning(bool isDown)
{
    _isServerRunning.store(isDown, std::memory_order_relaxed);
}

bool CaptureInfo::isRunning() const
{
    return _isServerRunning.load(std::memory_order_relaxed);
}
