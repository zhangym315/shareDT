#include "StringTools.h"

bool StringTools::isAllNumberString(const std::string & s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !::isdigit(c); }) == s.end();
}

/* Return false if failed */
bool StringTools::toInt(const std::string &input, int &out)
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
#include <vector>

LPCWSTR StringTools::stdString2LPCWSTR(const std::string & input) {
    std::wstring ws = std::wstring(input.begin(), input.end());
    LPCWSTR ret = ws.c_str();
    return ret;
}

LPWSTR StringTools::stdString2LPWSTR(const std::string & input) {
    int wstrLen = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, NULL, 0);
    if (wstrLen == 0)
        return nullptr;

    std::vector<wchar_t> buffer(wstrLen);
    MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, buffer.data(), wstrLen);

    LPWSTR lpwstr = buffer.data();
    return lpwstr;
}

#endif

std::string& operator<<(std::string & lhs, const std::string & rhs) {
    return lhs += rhs;
}

std::string& operator<<(std::string & lhs, const char * rhs) {
    return lhs += std::string(rhs);
}

std::string & operator<<(std::string & lhs,  int rhs) {
    return lhs += std::to_string(rhs);
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

