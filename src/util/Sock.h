#ifndef _SOCK_H_
#define _SOCK_H_

#ifdef __SHAREDT_WIN__
#include <WinSock2.h>
#include <Windows.h>
#endif

#include "StringTools.h"

#define LOCALHOST "127.0.0.1"
#define SHAREDT_INTERNAL_PORT_START 31400

class SocketFD
{
  public:
#ifdef __SHAREDT_WIN__
    SocketFD(HANDLE fd) : _fd(fd)
    {
    }
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

enum TypeSocket {BlockingSocket, NonBlockingSocket};

class Socket {
  public:
    virtual ~Socket();
    Socket(const Socket&);
    Socket& operator=(Socket&);

    String ReceiveLine();
    String ReceiveBytes();

    void   Close();

    void   SendLine (String);
    void   SendBytes(const String&);
    void   send(const char * buf) { SendBytes(String(buf)); }
    int    getSocket() { return s_; }

  protected:
    friend class SocketServer;
    friend class SocketSelect;

    Socket(SOCKET s);
    Socket();

    SOCKET s_;
    int* refCounter_;

  private:
    static void Start();
    static void End();
    static int  nofSockets_;
};

class SocketClient : public Socket {
  public:
    SocketClient(const String& host, int port);
    void write(const char * bytes) { SendBytes(bytes); }
};

class SocketServer : public Socket {
  public:
    SocketServer(int port, int connections, TypeSocket type=BlockingSocket);
    Socket* Accept();
    int  getPort() { return _port; }

  private:
    int _port;
};

class SocketSelect {
public:
    SocketSelect(Socket const * const s1, Socket const * const s2=NULL, TypeSocket type=BlockingSocket);
    bool Readable(Socket const * const s);

private:
    fd_set fds_;
};

#endif //_SOCK_H_
