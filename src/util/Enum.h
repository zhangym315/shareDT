#ifndef _ENUM_H_
#define _ENUM_H_

#include "TypeDef.h"


/*
 * The "0[arr]" gives the same result as "arr[0]" but it helpfully
 * causes a compile error if accidentally used with something
 * like an STL vector.
 */
#define ARRAY_ELEMENTS(arr)	(sizeof(arr) / sizeof(0[arr]))


extern const String INVALID_ENUM;
extern String enumToStr( int enumval, const char *const*array, size_t arrsz );

#define ENUM_TO_STR(val,ar) enumToStr((int)val, ar, ARRAY_ELEMENTS(ar))

#endif //_ENUM_H_
