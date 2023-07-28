/*********************************************************
 * StringTools.h                                         *
 *                                                       *
 * std::string related function                               *
 *                                                       *
 *********************************************************/


#ifndef _STRINGTOOLS_H_
#define _STRINGTOOLS_H_

#include "TypeDef.h"
#include <iostream>
#include <string>
#include <algorithm>

bool isNumber(const std::string & s);
std::string getString(char * x);
bool   toInt(const std::string & in, int &out);

std::string& operator<<(std::string & lhs, const std::string & rhs);
std::string& operator<<(std::string & lhs, const char * rhs);
std::string & operator<<(std::string & lhs,  int rhs);

#ifdef __SHAREDT_WIN__
#include <locale>
#include <codecvt>
#include <string>

std::wstring utf8_to_utf16(std::string utf8_string);
std::string utf16_to_utf8(std::wstring utf16_string);

#endif

#endif //_STRINGTOOLS_H_
