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

std::string Socket::receiveStrings() const
{
    char ret[BUFSIZE];
    int  reclen;
    if((reclen=::recv(_s, ret, BUFSIZE, MSG_PEEK)) == -1)
    {
            return "";
    }
    ret[reclen] = '\0';
    return ret;
}

std::string Socket::ReceiveLine() const
{
    std::string ret;
    while (1) {
        char r;

        switch(recv(_s, &r, 1, MSG_PEEK))
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

ssize_t Socket::receiveBytes(unsigned char *b, size_t s) const {
    return  ::recv(_s, (char *)b, s, MSG_WAITALL);
}

size_t Socket::sendStringLine(std::string s) const
{
    s += '\n';
    return ::send(_s,s.c_str(),s.length(),0);
}

size_t Socket::sendString(const std::string& s) const
{
    return ::send(_s,s.c_str(),s.length()+1,0);
}

size_t Socket::sendBytes(const unsigned char *p, size_t size) const {
    return ::send(_s, (const char *)p, size,0);
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
            LOGGER.error() << "Invalid Socket";
            return nullptr;
        }
    }

    Socket* r = new Socket(new_sock);
    return r;
}

SocketClient::SocketClient(const std::string& host, int port) : Socket(),
                    _tv{ _tv.tv_sec = 60, _tv.tv_usec = 0}
{
    std::string error;

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

    if (::setsockopt (_s, SOL_SOCKET, SO_RCVTIMEO, &_tv, sizeof(_tv)) < 0)
        LOGGER.warn() << "Failed to set timeout to seconds=" << _tv.tv_sec;
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