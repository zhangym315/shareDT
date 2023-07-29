#include "ReadWriteFD.h"

#include <fcntl.h>

#define BUFSIZE 5120

ReadWriteFD::ReadWriteFD(const char * path)
{
    ReadWriteFD(path, 0666);
}

ReadWriteFD::ReadWriteFD(const char * path, int oflag)
{
#ifndef __SHAREDT_WIN__
    ::open(path, oflag);
    _flag = oflag;
#else
    _fd = CreateNamedPipeA(path, PIPE_ACCESS_DUPLEX,
                           PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                           PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
#endif
    strcpy(_path, path);
}

char * ReadWriteFD::read()
{
#ifdef __SHAREDT_WIN__
    while(true) {
        Sleep(1000);
        DWORD total = 0;
        if(PeekNamedPipe(_fd, NULL, 0, NULL, &total, NULL))
            continue;
        if(total > 0) break;
    }
    ReadFile(_fd, _buf, BUFSIZE, NULL, NULL);
#else
    open(O_RDONLY);
    bzero(_buf, BUFSIZE);
    ::read(_fd, _buf, BUFSIZE);

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
    ::write(_fd, buf, strlen(buf)+1);
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
    _fd = ::open(_path, flag);
#endif
    _flag = flag;
}

void ReadWriteFD::close()
{
#ifdef __SHAREDT_WIN__
    CloseHandle(_fd);
#else
    ::close(_fd);
#endif
}
