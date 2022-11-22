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
#ifdef __SHAREDT_WIN__
#include <locale>
#include <codecvt>
#include <string>

std::wstring utf8_to_utf16(std::string utf8_string);
std::string utf16_to_utf8(std::wstring utf16_string);

#endif

class String {
public:
    explicit String(const char* data);
    explicit String(const std::string & data);
    explicit String(size_t init_size = 20);
    explicit String(const String &string);

    ~String();

    char* getText();
    void setText(const char* text);
    size_t getLength() const;
    void setLength(size_t size);
    void add(const String &text);

    template<typename T>
    String &operator<<(const T & s) {
        add(toString(s));
        return *this;
    }

    static String toString(int v) {
        return String(std::to_string(v));
    }

    static String toString(size_t v) {
        return String(std::to_string(v));
    }

    static String toString(const char * v) {
        return String(*v);
    }

private:
    char*  _buf;
    size_t _size;
};


#endif //_STRINGTOOLS_H_
