#ifndef _CGWINDOWPROCESSOR_H_
#define _CGWINDOWPROCESSOR_H_

#include <memory>
#include "ImageRect.h"
#include "WindowProcessor.h"

class CGFrameProcessor: public BaseFrameProcessor {
    CapMonitor SelectedMonitor;
  public:

    WINPROCESSOR_RETURN Init(CapWindow& window);
    WINPROCESSOR_RETURN ProcessFrame(const CapWindow& window);
};

#endif //_CGWINDOWPROCESSOR_H_
