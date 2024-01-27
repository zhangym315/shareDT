#ifndef _STRINGTOOLS_H_
#define _STRINGTOOLS_H_

#ifdef __SHAREDT_WIN__
#include <Windows.h>
#endif

#include <algorithm>
//#include "TypeDef.h"
#include <iostream>
#include <string>

class StringTools {
public:
    static bool isAllNumberString(const std::string & s);
    static bool toInt(const std::string & in, int &out);
#ifdef __SHAREDT_WIN__
    static LPCWSTR stdString2LPCWSTR(const std::string & input);
    static LPWSTR stdString2LPWSTR(const std::string & input);
#endif
};

std::string & operator<<(std::string & lhs, const std::string & rhs);
std::string & operator<<(std::string & lhs, const char * rhs);
std::string & operator<<(std::string & lhs,  int rhs);

#ifdef __SHAREDT_WIN__
#include <locale>
#include <codecvt>
#include <string>

std::wstring utf8_to_utf16(std::string utf8_string);
std::string utf16_to_utf8(std::wstring utf16_string);

#endif

#endif //_STRINGTOOLS_H_
