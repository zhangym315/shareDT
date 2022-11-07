/*********************************************************
 * StringTools.h                                         *
 *                                                       *
 * std::string related function                               *
 *                                                       *
 *********************************************************/


#ifndef _STRINGTOOLS_H_
#define _STRINGTOOLS_H_

#include <iostream>
#include <string>
#include <algorithm>
#include "TypeDef.h"

bool   isNumber(const std::string & s);
std::string getString(char * x);
bool   toInt(const std::string & in, int &out);
#ifdef __SHAREDT_WIN__
#include <locale>
#include <codecvt>
#include <string>

std::wstring utf8_to_utf16(std::string utf8_string);
std::string utf16_to_utf8(std::wstring utf16_string);

#endif

class String {
public:
    String(const char* data);
    String(size_t init_size = 20);
    String(const String &string);

    ~String();

    char* getText();
    void setText(const char* text);
    size_t getLength() const;
    void setLength(size_t size);
    void add(const String &text);

    template<typename T>
    static String toString(const T & s);
private:
    char*  _buf;
    size_t _size;

    template<typename T>
    friend String &operator<<(String &iostream, const T &string);
};


#endif //_STRINGTOOLS_H_
