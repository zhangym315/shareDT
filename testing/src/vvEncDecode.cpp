#include "vvEncDecode.h"
#include "vvenc/vvenc.h"
#include "vvdec/vvdec.h"

#include "gtest/gtest.h"
#include "RemoteGetter.h"
#include "RGByuv.h"
#include "ExportAll.h"

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

static vvdecDecoder* decoder = nullptr;
static vvdecParams params;
static vvdecAccessUnit* au = nullptr;
static int deframeCounter = 0;
static void deCode(vvencAccessUnit * encodedAU, FrameBuffer * fb)
{
    if (decoder == nullptr && au== nullptr) {
        vvdec_params_default( &params );
        params.logLevel = VVDEC_INFO;

        decoder = vvdec_decoder_open( &params );
        au = vvdec_accessUnit_alloc();
        vvdec_accessUnit_default( au );
        vvdec_accessUnit_alloc_payload( au, fb->getWidth() * fb->getHeight() *4 );
        deframeCounter = 0;
    }

    vvdecFrame* frame;
    int ret;

    if (encodedAU != nullptr) {
        memcpy( au->payload, encodedAU->payload, encodedAU->payloadUsedSize );
        au->payloadUsedSize = encodedAU->payloadUsedSize;

        frame = nullptr;
        ret   = vvdec_decode( decoder, au, &frame );
        if( ret != VVDEC_OK && ret != VVDEC_TRY_AGAIN ) {
            std::cerr << "Failed to decode frame!!" << std::endl;
        }
        if ( frame ) {
            std::cout << "Successfully decode frame, deframeCounter=" << deframeCounter++ << std::endl;
            vvdec_frame_unref( decoder, frame );
        }
    } else {
        do {
            frame = nullptr;
            ret = vvdec_flush( decoder, &frame );
            if( ret != VVDEC_OK && ret != VVDEC_EOF ) {
                std::cerr << "Failed flush to decode frame!!" << std::endl;
                return;
            }
            if( frame ) {
                std::cout << "Successfully flush decode frame, deframeCounter=" << deframeCounter++ << std::endl;
                vvdec_frame_unref( decoder, frame );
            }
        } while (ret != VVDEC_EOF);
    }
}

TEST(vv_encode_decode, vv_encode) {
    VVFrameGetter fg;
    EXPECT_TRUE(fg.valid());

    FrameBuffer * fb;
    if ((fb=fg.getFrame()) == nullptr) {
        std::cerr << "Failed to get frame" << std::endl;
        return;
    }
//    fb->getSize();

    /**************** for testing     */
    vvenc_config vvenccfg;
    vvenccfg.m_internChromaFormat = VVENC_CHROMA_420;
    vvenc_init_default( &vvenccfg, fb->getWidth(), fb->getHeight(), 10,
                        VVENC_RC_OFF, VVENC_AUTO_QP,
                        vvencPresetMode::VVENC_MEDIUM );
    vvenccfg.m_verbosity = VVENC_DETAILS;
    vvenccfg.m_numThreads = 20;
    vvenccfg.m_framesToBeEncoded = 25;
    vvenc_set_msg_callback( &vvenccfg, NULL, &msgFnc );

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

    std::cout << "Getting width=" << fb->getWidth() << " height=" << fb->getHeight() << std::endl;
    // --- allocate memory for output packets
    vvencAccessUnit AU;
    vvenc_accessUnit_default( &AU );
//    const int auSizeScale = vvenccfg.m_internChromaFormat <= VVENC_CHROMA_420 ? 2 : 3;
    vvenc_accessUnit_alloc_payload( &AU,  vvenccfg.m_SourceWidth * vvenccfg.m_SourceHeight + 1024 );

    // --- allocate memory for YUV input picture
    vvencYUVBuffer cYUVInputBuffer;
    vvenc_YUVBuffer_default( &cYUVInputBuffer );
    vvenc_YUVBuffer_alloc_buffer( &cYUVInputBuffer, vvenccfg.m_internChromaFormat, fb->getWidth(), fb->getHeight() );

    /**************** end for testing */
    int64_t seq = 0;
    bool bEncodeDone = false;
    for (int round=0; round < 10; round ++) {

        if (decoder!=nullptr || au!= nullptr) {
            if (au) vvdec_accessUnit_free( au );
            if (decoder) vvdec_decoder_close( decoder );
            decoder = nullptr;
            au = nullptr;
        }

        for (int i=0; i < 25; i++) {
    //        ExportAll::writeToFile(std::string("IMG_") + std::to_string(i) + std::string(".png"), fb);

            RGByuv::rgb32Toyuv42016Bit(fb->getWidth(), fb->getHeight(),
                                  (const uint8_t *)fb->getData(), fb->getWidth()*4,
                                  cYUVInputBuffer.planes[0].ptr,
                                  cYUVInputBuffer.planes[1].ptr,
                                  cYUVInputBuffer.planes[2].ptr,
                                  cYUVInputBuffer.planes[0].stride,
                                  cYUVInputBuffer.planes[1].stride,
                                  YCbCrType::YCBCR_JPEG);
            cYUVInputBuffer.sequenceNumber  = i;
    //        cYUVInputBuffer.cts             =  (int64_t)cYUVInputBuffer.sequenceNumber;
    //        cYUVInputBuffer.ctsValid        = true;
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

            std::cout << "AU.payloadUsedSize=" << AU.payloadUsedSize << " AU.payloadSize:" << AU.payloadSize << std::endl;

            std::cout << "Getting Frame number=" << i << std::endl;

            if (AU.payloadUsedSize > 0) deCode(&AU, fb);

            fb->setUsed();
            auto startTime = std::chrono::steady_clock::now();
            if ((fb = fg.getFrame()) == nullptr) {
                std::cout << "Failed to get frame buffer" << std::endl;
            }
        }

        deCode(nullptr, fb); // flush
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
//    _c->getScreenProvide()->setTargetImageType(SPImageType::SP_IMAGE_RGBA);
    _c->getScreenProvide()->sampleResume();

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
