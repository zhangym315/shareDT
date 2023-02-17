#include "vvEncDecode.h"
#include "vvenc/vvenc.h"

#include "gtest/gtest.h"
#include "RemoteGetter.h"
#include "RGByuv.h"

/************************ Starts unused symbols ***************************************/

const char * SHAREDT_SERVER_SVCNAME            = "shareDTServer";
const char * SHAREDT_SERVER_COMMAND_START      = "start";
const char * SHAREDT_SERVER_COMMAND_STOP       = "stop";
const char * SHAREDT_SERVER_COMMAND_RESTART    = "restart";
const char * SHAREDT_SERVER_COMMAND_CAPTURE    = "capture";
const char * SHAREDT_SERVER_COMMAND_NEWCAPTURE = "newCapture";
const char * SHAREDT_SERVER_COMMAND_SHOW       = "show";
const char * SHAREDT_SERVER_COMMAND_STATUS     = "status";
const char * SHAREDT_SERVER_COMMAND_EXPORT     = "export";
const char * SHAREDT_SERVER_COMMAND_NODAEMON   = "nodaemon";
const char * SHAREDT_SERVER_COMMAND_DISPLAY    = "display";
const char * SHAREDT_SERVER_COMMAND_CONNECT    = "connect";
const char * SHAREDT_SERVER_COMMAND_GET        = "get";
const char * SHAREDT_SERVER_COMMAND_REMOTGET   = "remoteGet";
void RemoteGetter::send(){ }

/************************ End symbols *************************************************/
void msgFnc( void*, int level, const char* fmt, va_list args )
{
    vfprintf( level == 1 ? stderr : stdout, fmt, args );
}

TEST(vv_encode_decode, vv_encode) {
    VVFrameGetter fg;
    EXPECT_TRUE(fg.valid());

    FrameBuffer * fb = fg.getFrame();
    fb->getSize();

    /**************** for testing     */
    vvenc_config vvenccfg;
    vvenccfg.m_internChromaFormat = VVENC_CHROMA_422;
    vvenc_init_default( &vvenccfg, fb->getWidth(), fb->getHeight(), 10,
                        VVENC_RC_OFF, VVENC_AUTO_QP,
                        vvencPresetMode::VVENC_MEDIUM );

    // initialize the encoder
    vvencEncoder *enc = vvenc_encoder_create();
    if( nullptr == enc )
    {
        return ;
    }
    int iRet = vvenc_encoder_open( enc, &vvenccfg );
    if( 0 != iRet )
    {
//        std::cout <<  "vvencapp cannot create encoder" << iRet << vvenc_get_last_error( enc );
        vvenc_encoder_close( enc );
        return ;
    }

    // --- allocate memory for output packets
    vvencAccessUnit AU;
    vvenc_accessUnit_default( &AU );
    const int auSizeScale = vvenccfg.m_internChromaFormat <= VVENC_CHROMA_420 ? 2 : 3;
    vvenc_accessUnit_alloc_payload( &AU, auSizeScale * vvenccfg.m_SourceWidth * vvenccfg.m_SourceHeight + 1024 );

    // --- allocate memory for YUV input picture
    vvencYUVBuffer cYUVInputBuffer;
    vvenc_YUVBuffer_default( &cYUVInputBuffer );
    vvenc_YUVBuffer_alloc_buffer( &cYUVInputBuffer, vvenccfg.m_internChromaFormat, fb->getWidth(), fb->getHeight() );

    /**************** end for testing */
    size_t seq = 0;
    bool bEncodeDone = false;

    for (int i=0; i < 100; i++) {
/*        if ((fb = fg.getFrame()) == nullptr) {
            fb->setUsed();
            continue;
        }
*/
        RGByuv::rgb32Toyuv420(fb->getWidth(), fb->getHeight(),
                              (const uint8_t *)fb->getData(), fb->getWidth()*4,
                              (uint8_t *) cYUVInputBuffer.planes[0].ptr, (uint8_t *) cYUVInputBuffer.planes[1].ptr, (uint8_t *) cYUVInputBuffer.planes[2].ptr,
                              cYUVInputBuffer.planes[0].stride, cYUVInputBuffer.planes[1].stride,
                              YCbCrType::YCBCR_JPEG);
//        fb->setUsed();
        cYUVInputBuffer.sequenceNumber  = seq++;
        cYUVInputBuffer.cts             =  (int64_t)vvenccfg.m_FrameRate;
        cYUVInputBuffer.ctsValid        = true;
        auto * ptrYUVInputBuffer               = &cYUVInputBuffer;
        // call encode
        iRet = vvenc_encode( enc, ptrYUVInputBuffer, &AU, &bEncodeDone );
        if( 0 != iRet )
        {
            std::cout << "encoding failed" << iRet << std::endl;
            vvenc_YUVBuffer_free_buffer( &cYUVInputBuffer );
            vvenc_accessUnit_free_payload( &AU );
            vvenc_encoder_close( enc );
            return ;
        }

        std::cout << "AU.payloadUsedSize=" << AU.payloadUsedSize << std::endl;
    }
}

VVENC_DEC_Test::VVENC_DEC_Test() {

}

VVFrameGetter::VVFrameGetter() : valid_(false), _c(nullptr) {
    MonitorVectorProvider mvp(true);
    if (mvp.get().empty()) {
        std::cerr << __FUNCTION__  << " Failed to get monitor for testing" << std::endl;
        return;
    }

    _c = std::make_unique<Capture>(SP_MONITOR, mvp.get()[0].getId(), "VVTesting");

    if (_c->initSrceenProvider() != RETURN_CODE_SUCCESS ||
        _c->getScreenProvide() == nullptr){
        LOGGER.error() << "Failed to start server";
        return ;
    }

    if(!_c->getScreenProvide()->startSample()) {
        LOGGER.error() << "Failed to start SampleProvider" ;
        return ;
    }

    while ( !_c->getScreenProvide()->isSampleReady() ) {
        std::this_thread::sleep_for(50ms);
    }
    _c->getScreenProvide()->sampleResume();

    _c->getScreenProvide()->setTargetImageType(SPImageType::SP_IMAGE_YUV420);
    valid_ = true;
}


FrameBuffer * VVFrameGetter::getFrame() {
    FrameBuffer * fb;
    std::chrono::microseconds duration(500);

    for (int i=0; i<20; i++ )
    {
        if ( (fb = getSreenProvider()->getFrameBuffer()) == nullptr ||
             (fb->getData() == nullptr)) {
            getSreenProvider()->sampleResume();
            std::this_thread::sleep_for(duration);
            continue;
        }
        break;
    }

    return fb;
}
