#ifndef SHAREDT_REMOTEGETTER_H
#define SHAREDT_REMOTEGETTER_H
#include "Sock.h"

struct RemoteGetterMsg {
    RemoteGetterMsg()  = default;
    RemoteGetterMsg(size_t width, size_t height) : w(width), h(height), dataLen(0), name{0}, cmdArgs{0} { }

    void convert();

    size_t w;
    size_t h;
    size_t dataLen;

    char name[64];
    char cmdArgs[128];
};

class RemoteGetter {
public:
    explicit RemoteGetter(Socket * sk) : _sk(sk), _replyW(140), _replyH(120) { }

    void send();

private:
    Socket * _sk;
    size_t _replyW;
    size_t _replyH;
};

#endif //SHAREDT_REMOTEGETTER_H
