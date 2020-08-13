#include <unistd.h>
#include <sys/socket.h>

#include "Logger.h"
#include "Sock.h"

void Socket::close()
{
    ::close(_fd);
}

int Socket::send(const char * buf)
{
    int rc = ::send(_fd, buf, strlen(buf), 0);
    if (rc < -1) {
    }
    return rc;
}

int Socket::recv(char * buf, size_t size)
{
    int rc = ::recv(_fd, buf, size-1, MSG_WAITALL);
    if (rc == -1) {
        LOGGER.noPre("RECV ERROR");
    }
    buf[rc] = '\0';
    return rc;
}

