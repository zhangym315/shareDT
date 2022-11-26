#ifndef SHAREDT_REMOTEGETTER_H
#define SHAREDT_REMOTEGETTER_H
#include "Sock.h"

struct RemoteGetterMsg {
    RemoteGetterMsg()  = default;
    RemoteGetterMsg(uint32_t width, uint32_t height) : w(width), h(height), dataLen(0), name{0}, cmdArgs{0} { }

    void convert();

    uint32_t w;
    uint32_t h;
    uint32_t dataLen;

    char name[64];
    char cmdArgs[128];
};

class RemoteGetter {
public:
    explicit RemoteGetter(Socket * sk) : _sk(sk), _replyW(140), _replyH(120) { }

    void send();

private:
    Socket * _sk;
    uint32_t _replyW;
    uint32_t _replyH;
};

#endif //SHAREDT_REMOTEGETTER_H
