#ifndef _SOCK_H_
#define _SOCK_H_

#ifdef __SHAREDT_WIN__
#include <windows.h>
#endif

class SocketFD
{
  public:
#ifdef __SHAREDT_WIN__
    SocketFD(HANDLE fd) : _fd(fd) { }
#else
    SocketFD(int fd) : _fd (fd) { }
#endif

    void close();
    int send(const char * buf);
    int recv(char * buf, size_t size);

  private:
    SocketFD() { } // not allowed default constructor
#ifdef __SHAREDT_WIN__
    HANDLE _fd;
#else
    int _fd;
#endif
    bool _isServer;
};

#endif //_SOCK_H_
