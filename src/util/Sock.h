#ifndef _SOCK_H_
#define _SOCK_H_

#ifdef __SHAREDT_WIN__
#include <WinSock2.h>
#include <Windows.h>
typedef size_t ssize_t;
#else
#define SOCKET int
#define INVALID_SOCKET -1
#endif

#include "StringTools.h"

#define LOCALHOST "127.0.0.1"
#define SHAREDT_INTERNAL_PORT_START 31400

enum TypeSocket {BlockingSocket, NonBlockingSocket};

class SocketFD
{
public:
#ifdef __SHAREDT_WIN__
    SocketFD(HANDLE fd) : _fd(fd) { }
#else
    SocketFD(int fd) : _fd (fd) { }
#endif

    void close() const;
    int send(const char * buf) const;
    int recv(char * buf, size_t size) const;

private:
    SocketFD() { }
#ifdef __SHAREDT_WIN__
    HANDLE _fd;
#else
    int _fd;
#endif
};

class Socket {
public:
    virtual ~Socket();
    Socket(const Socket&);

    std::string ReceiveLine() const;
    std::string receiveStrings() const;

    ssize_t receiveBytes(unsigned char * b, size_t s) const;

    void   Close() const;

    size_t   sendBytes(const unsigned char * p, size_t size) const;
    size_t   sendStringLine (std::string) const;
    size_t   sendString(const std::string&) const;
    size_t   send(const char * buf) { return sendString(std::string(buf)); }
    SOCKET getSocket() { return _s; }

protected:
    friend class SocketServer;
    friend class SocketSelect;

    Socket(SOCKET s);
    Socket();

    SOCKET _s;

private:
    static void Start();
    static void End();
    static int  _nofSockets;
};

class SocketClient : public Socket {
public:
    SocketClient(const std::string& host, int port);
    void write(const char * bytes) { sendString(bytes); }
#ifndef __SHAREDT_WIN__
    int connectWait (int sockno, struct sockaddr * addr, size_t addrlen, struct timeval * timeout);
#endif

private:
    struct timeval _tv;
    bool   _isConnected;
};

class SocketServer : public Socket {
  public:
    SocketServer(int port, int connections, TypeSocket type=BlockingSocket);

    /* caller should take ownership of return variable to release */
    Socket* accept();
    int  getPort() const { return _port; }
    bool isInit() const { return _isInit; }

  private:
    bool _isInit;
    int  _port;
};

#endif //_SOCK_H_
