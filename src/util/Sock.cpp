#include "Sock.h"
#include "Logger.h"
#ifdef __SHAREDT_WIN__
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#endif

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

using namespace std;

int Socket::nofSockets_= 0;

void Socket::Start()
{
#ifdef __SHAREDT_WIN__
    if (!nofSockets_)
    {
        WSADATA info;
        if (WSAStartup(MAKEWORD(2,0), &info))
        {
            throw "Could not start WSA";
        }
    }
    ++nofSockets_;
#endif
}

void Socket::End()
{
#ifdef __SHAREDT_WIN__
    WSACleanup();
#endif
}

Socket::Socket() : s_(0)
{
    Start();
    // UDP: use SOCK_DGRAM instead of SOCK_STREAM
    s_ = socket(AF_INET,SOCK_STREAM,0);

    if (s_ == INVALID_SOCKET)
    {
        throw "INVALID_SOCKET";
    }

    refCounter_ = new int(1);
}

Socket::Socket(SOCKET s) : s_(s)
{
    Start();
    refCounter_ = new int(1);
};

Socket::~Socket()
{
    if (! --(*refCounter_))
    {
        Close();
        delete refCounter_;
    }

    --nofSockets_;
    if (!nofSockets_) End();
}

Socket::Socket(const Socket& o)
{
    refCounter_=o.refCounter_;
    (*refCounter_)++;
    s_         =o.s_;

    nofSockets_++;
}

Socket& Socket::operator=(Socket& o)
{
    (*o.refCounter_)++;

    refCounter_=o.refCounter_;
    s_         =o.s_;

    nofSockets_++;

    return *this;
}

void Socket::Close()
{
    closesocket(s_);
}

String Socket::ReceiveBytes()
{
    char ret[BUFSIZE];
    if(recv(s_, ret, BUFSIZE, 0) == -1)
    {
            return "";
    }
    return ret;
}

String Socket::ReceiveLine()
{
    String ret;
    while (1) {
        char r;

        switch(recv(s_, &r, 1, 0)) {
            case 0:
                return ret;
            case -1:
                return "";

        }

        ret += r;
        if (r == '\n')  return ret;
    }
}

void Socket::SendLine(String s)
{
    s += '\n';
    ::send(s_,s.c_str(),s.length(),0);
}

void Socket::SendBytes(const String& s)
{
    ::send(s_,s.c_str(),s.length(),0);
}

SocketServer::SocketServer(int port, int connections, TypeSocket type)
{
    sockaddr_in sa;

    s_ = socket(AF_INET, SOCK_STREAM, 0);
    if (s_ == INVALID_SOCKET) {
        throw "INVALID_SOCKET";
    }

    if(type==NonBlockingSocket) {
        u_long arg = 1;
#ifdef __SHAREDT_WIN__
        ioctlsocket(s_, FIONBIO, &arg);
#else
        ioctl(s_, O_NONBLOCK, &arg);
#endif
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = PF_INET;
    sa.sin_port = htons(port);

    int bindRet = ::bind(s_, (sockaddr *)&sa, sizeof(sockaddr_in));
    int retry = 0;
    while(bindRet &&
#ifdef __SHAREDT_WIN__
    WSAGetLastError() == WSAEADDRINUSE &&
#endif
            (retry++) < 200)
    {
        LOGGER.warn() << "Port has been used, retry another one for internal communication: " << port;

#ifdef __SHAREDT_WIN__
        Sleep(500);
#else
        sleep(1);
#endif
        sa.sin_port = htons(++port);;
        bindRet = ::bind(s_, (sockaddr *)&sa, sizeof(sockaddr_in));
    }

#ifdef __SHAREDT_WIN__
    if (bindRet == SOCKET_ERROR)
#else
    if (bindRet == -1)
#endif
    {
        closesocket(s_);
        throw("INVALID_SOCKET");
    }

    _port = port;
    listen(s_, connections);
}

Socket* SocketServer::Accept()
{
    SOCKET new_sock = ::accept(s_, 0, 0);
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

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((in_addr *)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    if (::connect(s_, (sockaddr *) &addr, sizeof(sockaddr))) {
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
    FD_SET(const_cast<Socket*>(s1)->s_,&fds_);
    if(s2) {
        FD_SET(const_cast<Socket*>(s2)->s_,&fds_);
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
    if (FD_ISSET(s->s_,&fds_)) return true;
    return false;
}