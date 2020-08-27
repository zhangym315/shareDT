#include "ReadWriteFD.h"

#include <fcntl.h>

#define BUFSIZE 5120

ReadWriteFD::ReadWriteFD(const char * path)
{
#ifdef __SHAREDT_WIN__
    _fd = CreateNamedPipe(path, PIPE_ACCESS_DUPLEX,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                    PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);

#else
    ReadWriteFD(path, 0666);
#endif
}

ReadWriteFD::ReadWriteFD(const char * path, int oflag)
{
#ifndef __SHAREDT_WIN__
    OS_OPEN(path, oflag);
    strcpy(_path, path);
    _flag = oflag;
#endif
}

char * ReadWriteFD::read()
{
#ifdef __SHAREDT_WIN__
    ReadFile(_fd, _buf, BUFSIZE, NULL, NULL);
#else
    open(O_RDONLY);
    bzero(_buf, MAX_BUF);
    OS_READ(_fd, _buf, MAX_BUF);

    close();
#endif
    return _buf;
}

void ReadWriteFD::write(const char * buf)
{
#ifdef __SHAREDT_WIN__
    WriteFile( _fd, buf, strlen(buf)+1, NULL, NULL);
#else
    open(O_WRONLY);
    OS_WRITE(_fd, buf, strlen(buf)+1);
    close();
#endif
}

void ReadWriteFD::open()
{
    open(_flag);
}

void ReadWriteFD::open(int flag)
{
#ifdef __SHAREDT_WIN__
#else
    _fd = OS_OPEN(_path, flag);
#endif
    _flag = flag;
}

void ReadWriteFD::close()
{
    OS_CLOSE(_fd);
}
