#ifdef __SHAREDT_WIN__
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif

#include "Logger.h"
#include "Sock.h"

void SocketFD::close()
{
#ifdef __SHAREDT_WIN__
    CloseHandle(_fd);
#else
    ::close(_fd);
#endif
}

int SocketFD::send(const char * buf)
{
    int rc;
#ifdef __SHAREDT_WIN__
    rc = WriteFile( _fd, buf, strlen(buf)+1, NULL, NULL);
#else
    rc = ::send(_fd, buf, strlen(buf)+1, 0);
    if (rc < -1) {
    }
#endif
    return rc;
}

int SocketFD::recv(char * buf, size_t size)
{
    int rc;
#ifdef __SHAREDT_WIN__
    rc = ReadFile(_fd, buf, size, NULL, NULL);
#else
    rc = ::recv(_fd, buf, size-1, MSG_WAITALL);
    if (rc == -1) {
        LOGGER.noPre("RECV ERROR");
    }
    buf[rc] = '\0';
#endif
    return rc;
}

