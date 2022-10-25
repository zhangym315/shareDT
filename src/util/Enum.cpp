#include "Enum.h"
#include <string>

const std::string INVALID_ENUM = "INVALID_TYPE=";

std::string enumToStr( int enumval, const char *const * array, size_t arrsz )
{
    if ( enumval >= 0 && enumval < (int)arrsz ) {
        return array[ enumval ];
    }
    return INVALID_ENUM + std::to_string(enumval) ;
}
