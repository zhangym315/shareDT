#include "Converter.h"
#include <cinttypes>
#include <bit>

template uint16_t Converter::toLittleEndian(uint16_t);
template uint32_t Converter::toLittleEndian(uint32_t);
template unsigned long Converter::toLittleEndian(unsigned long);

const bool isLittleEndian = (std::endian::native == std::endian::little);

uint16_t swapImpl(uint16_t v){
    return ((v & 0xFF) << 8) | ((v & 0xFF00) >> 8);
}

uint32_t swapImpl(uint32_t v){
    return ((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24);
}

unsigned long swapImpl(unsigned long v) {
    return (unsigned long) swapImpl((uint32_t) v);
}

template<typename T>
T Converter::toLittleEndian(T x) {
        if (isLittleEndian) {
            return x;
        } else {
            return swapImpl(x);
        }
}
