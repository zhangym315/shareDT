#include "StringTools.h"
#include <type_traits>
#include <cstring>

bool isNumber(const std::string & s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !::isdigit(c); }) == s.end();
}

std::string getString(char * x)
{
    std::string s(x);
    return s;
}

/* Return false if failed */
bool toInt(const std::string &input, int &out)
{
    /* convert to int */
    try
    {
        out = std::stoi(input);
    }
    catch (std::invalid_argument const &e)
    {
        std::cout << "Bad input: invalid argument" << '\n';
        return false;
    }
    catch (std::out_of_range const &e)
    {
        std::cout << "Integer overflow: std::out_of_range thrown" << '\n';
        return false;
    }

    return true;
}

#ifdef __SHAREDT_WIN__
std::wstring utf8_to_utf16(std::string utf8_string)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(utf8_string);
}

std::string utf16_to_utf8(std::wstring utf16_string)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(utf16_string);
}
#endif


/*
 * String Class
 */
String::String(size_t init_size) {
    _buf = new char[init_size];
    _size = init_size;
    if (_buf != nullptr) {
        for (size_t i = 0; i < init_size; i++) {
            _buf[i] = '\0';
        }
    }
}

String::String(const char* buf) {
    size_t size = strlen(buf);

    _buf = new char[size];
    _size = size;

    if (_buf != nullptr) {
        strncpy(_buf, _buf, size);
    }
}

String::String(const String &string) {
    _size = string.getLength();
    _buf = new char[_size];

    if (_buf != nullptr) {
        strncpy(_buf, string._buf, _size);
    }
}

String::~String() {
    if (_buf != nullptr)
        delete[] _buf;
}

char* String::getText() {
    return _buf;
}

void String::setText(const char* text) {
    delete[] _buf;

    _size = strlen(text);
    _buf = new char[_size];

    strncpy(_buf, text, _size);
}

size_t String::getLength() const {
    return strlen(_buf);
}

void String::setLength(size_t size) {
    size_t old_length = getLength();
    char* old_data = _buf;

    _size = size;
    _buf = new char[size];

    for (size_t i = 0; i < size; i++) {
        if (i < old_length) {
            _buf[i] = old_data[i];
        } else {
            _buf[i] = '\0';
        }
    }

    delete[] old_data;

    _buf[size] = '\0';
}

void String::add(const String &text) {
    size_t new_size = _size + text._size;
    setLength(new_size);

    size_t length = getLength();
    for (size_t i = length; i < new_size; i++) {
        _buf[i] = text._buf[i - length];
    }
}

template<typename T>
String & operator<<(String &s, const T &d) {
    s.add(String::toString(d));
    return s;
}

template<typename T>
String String::toString(const T &s) {
    return String(std::to_string(s));
}
