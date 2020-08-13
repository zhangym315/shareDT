/*********************************************************
 * StringTools.h                                         *
 *                                                       *
 * String related function                               *
 *                                                       *
 *********************************************************/


#ifndef _STRINGTOOLS_H_
#define _STRINGTOOLS_H_

#include <iostream>
#include <string>
#include <algorithm>
#include "TypeDef.h"

bool   isNumber(const String & s);
String getString(char * x);
bool   toInt(const String & in, int &out);


#endif //_STRINGTOOLS_H_
