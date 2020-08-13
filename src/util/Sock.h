#ifndef _SOCK_H_
#define _SOCK_H_

class Socket
{
  public:
    Socket(int fd) : _fd (fd) { }

    void close();
    int send(const char * buf);
    int recv(char * buf, size_t size);

  private:
    Socket() { } // not allowed default constructor
    int _fd;
    bool _isServer;
};

#endif //_SOCK_H_
