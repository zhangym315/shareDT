#ifndef SHAREDT_CONVERTER_H
#define SHAREDT_CONVERTER_H

template<typename T>
extern T toLittleEndian(T x) ;
const extern bool isLittleEndian;

class Converter {
public:
    template<typename T>
    static T toLittleEndian(T x);
};

#endif //SHAREDT_CONVERTER_H
