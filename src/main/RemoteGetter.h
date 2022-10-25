#ifndef SHAREDT_REMOTEGETTER_H
#define SHAREDT_REMOTEGETTER_H
#include "Sock.h"

class RemoteGetter {
public:
    explicit RemoteGetter(Socket * sk) : _sk(sk) { }

    void send();

private:
    Socket * _sk;
};

#endif //SHAREDT_REMOTEGETTER_H
