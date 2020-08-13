#include "StringTools.h"

bool isNumber(const String & s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

String getString(char * x)
{
    String s(x);
    return s;
}

/* Return false if failed */
bool toInt(const String &input, int &out)
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
