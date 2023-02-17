#ifndef SHAREDT_VVENCDECODE_H
#define SHAREDT_VVENCDECODE_H
#include "Capture.h"

class VVFrameGetter final  {
public:
    VVFrameGetter();

    ScreenProvider * getSreenProvider() { return _c->getScreenProvide(); }
    FrameBuffer * getFrame();
    bool valid() const { return valid_; }
private:
    bool valid_;
    std::unique_ptr<Capture> _c;
};


class VVENC_DEC_Test {
public:
    VVENC_DEC_Test();

private:

};

#endif //SHAREDT_VVENCDECODE_H
