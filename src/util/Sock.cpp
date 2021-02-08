#include "Sock.h"
#include "Logger.h"

#ifndef __SHAREDT_WIN__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cerrno>
#include <netdb.h>
#include <sys/select.h>
#endif

using namespace std;

void SocketFD::close() const
{
#ifdef __SHAREDT_WIN__
    CloseHandle(_fd);
#else
    ::close(_fd);
#endif
}

int SocketFD::send(const char * buf) const
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

int SocketFD::recv(char * buf, size_t size) const
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

int Socket::_nofSockets= 0;

void Socket::Start()
{
#ifdef __SHAREDT_WIN__
    if (!_nofSockets)
    {
        WSADATA info;
        if (WSAStartup(MAKEWORD(2,0), &info))
        {
            throw "Could not start WSA";
        }
    }
    ++_nofSockets;
#endif
}

void Socket::End()
{
#ifdef __SHAREDT_WIN__
    WSACleanup();
#endif
}

Socket::Socket() : _s(0)
{
    Start();
    // UDP: use SOCK_DGRAM instead of SOCK_STREAM
    _s = socket(AF_INET, SOCK_STREAM, 0);

    if (_s == INVALID_SOCKET)
    {
        throw "INVALID_SOCKET";
    }

    _refCounter = new int(1);
}

Socket::Socket(SOCKET s) : _s(s)
{
    Start();
    _refCounter = new int(1);
};

Socket::~Socket()
{
    if (! --(*_refCounter))
    {
        Close();
        delete _refCounter;
    }

    --_nofSockets;
    if (!_nofSockets) End();
}

Socket::Socket(const Socket& o)
{
    _refCounter=o._refCounter;
    (*_refCounter)++;
    _s         =o._s;

    _nofSockets++;
}

Socket& Socket::operator=(Socket& o)
{
    (*o._refCounter)++;

    _refCounter=o._refCounter;
    _s         =o._s;

    _nofSockets++;

    return *this;
}

void Socket::Close() const
{
#ifdef __SHAREDT_WIN__
    closesocket(_s);
#else
    close(_s);
#endif
}

String Socket::ReceiveBytes() const
{
    char ret[BUFSIZE];
    int  reclen;
    if((reclen=::recv(_s, ret, BUFSIZE, 0)) == -1)
    {
            return "";
    }
    ret[reclen] = '\0';
    return ret;
}

String Socket::ReceiveLine() const
{
    String ret;
    while (1) {
        char r;

        switch(recv(_s, &r, 1, 0))
        {
            case 0:
                return ret;
            case -1:
                return "";

        }

        ret += r;
        if (r == '\n')  return ret;
    }
}

void Socket::SendLine(String s) const
{
    s += '\n';
    ::send(_s,s.c_str(),s.length(),0);
}

void Socket::SendBytes(const String& s) const
{
    ::send(_s,s.c_str(),s.length(),0);
}

SocketServer::SocketServer(int port, int connections, TypeSocket type)
{
    sockaddr_in sa{};

    _s = socket(AF_INET, SOCK_STREAM, 0);
    if (_s == INVALID_SOCKET) {
        throw "INVALID_SOCKET";
    }

    if(type==NonBlockingSocket) {
        u_long arg = 1;
#ifdef __SHAREDT_WIN__
        ioctlsocket(_s, FIONBIO, &arg);
#else
        ioctl(_s, O_NONBLOCK, &arg);
#endif
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = PF_INET;
    sa.sin_port = htons(port);

    int bindRet = ::bind(_s, (sockaddr *)&sa, sizeof(sockaddr_in));
    int retry = 0;
    while(bindRet &&
#ifdef __SHAREDT_WIN__
    WSAGetLastError() == WSAEADDRINUSE &&
#endif
            (retry++) < 200)
    {
        LOGGER.warn() << "Port("<< port << ") has been used, retry another one for internal communication: " << port+1;

        std::this_thread::sleep_for(500ms);
        sa.sin_port = htons(++port);;
        bindRet = ::bind(_s, (sockaddr *)&sa, sizeof(sockaddr_in));
    }

#ifdef __SHAREDT_WIN__
    if (bindRet == SOCKET_ERROR)
#else
    if (bindRet == -1)
#endif
    {
#ifdef __SHAREDT_WIN__
        closesocket(_s);
#else
        close(_s);
#endif
        throw("INVALID_SOCKET");
    }

    _port = port;
    listen(_s, connections);
}

Socket* SocketServer::Accept()
{
    SOCKET new_sock = ::accept(_s, nullptr, nullptr);
    if (new_sock == INVALID_SOCKET) {
#ifdef __SHAREDT_WIN__
        int rc = WSAGetLastError();
        if(rc==WSAEWOULDBLOCK) {
            return 0; // non-blocking call, no request pending
        }
        else
#endif
        {
            throw "Invalid Socket";
        }
    }

    Socket* r = new Socket(new_sock);
    return r;
}

SocketClient::SocketClient(const String& host, int port) : Socket()
{
    String error;

    hostent *he;
    if ((he = gethostbyname(host.c_str())) == 0) {
        error = strerror(errno);
        throw error;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((in_addr *)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    if (::connect(_s, (sockaddr *) &addr, sizeof(sockaddr))) {
#ifdef __SHAREDT_WIN__
        error = strerror(WSAGetLastError());
#else
        error = "connect error";
#endif
        throw error;
    }
}

SocketSelect::SocketSelect(Socket const * const s1, Socket const * const s2, TypeSocket type)
{
    FD_ZERO(&fds_);
    FD_SET(const_cast<Socket*>(s1)->_s,&fds_);
    if(s2) {
        FD_SET(const_cast<Socket*>(s2)->_s,&fds_);
    }

#ifdef __SHAREDT_WIN__
    TIMEVAL tval;
    TIMEVAL *ptval;
#else
    timeval tval;
    timeval *ptval;
#endif
    tval.tv_sec  = 0;
    tval.tv_usec = 1;

    if(type==NonBlockingSocket) {
        ptval = &tval;
    }
    else {
        ptval = 0;
    }

    if (select (0, &fds_, (fd_set*) 0, (fd_set*) 0, ptval)
#ifdef __SHAREDT_WIN__
        == SOCKET_ERROR)
#else
        == -1)
#endif
        throw "Error in select";
}

bool SocketSelect::Readable(Socket const* const s)
{
    if (FD_ISSET(s->_s,&fds_)) return true;
    return false;
}