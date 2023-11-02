#ifndef SHAREDT_RGBYUV_H
#define SHAREDT_RGBYUV_H
#include <cstdint>
typedef enum
{
    YCBCR_JPEG,
    YCBCR_601,
    YCBCR_709
} YCbCrType;


class RGByuv {
public:
    static void rgb24Toyuv420(
                        uint32_t width, uint32_t height,
                        const uint8_t *RGB, uint32_t RGB_stride,
                        uint8_t *Y, uint8_t *U, uint8_t *V,
                        uint32_t Y_stride, uint32_t UV_stride,
                        YCbCrType yuv_type);

    static void rgb32Toyuv420(
                        uint32_t width, uint32_t height,
                        const uint8_t *RGBA, uint32_t RGBA_stride,
                        uint8_t *Y, uint8_t *U, uint8_t *V,
                        uint32_t Y_stride, uint32_t UV_stride,
                        YCbCrType yuv_type);
};

#endif //SHAREDT_RGBYUV_H
