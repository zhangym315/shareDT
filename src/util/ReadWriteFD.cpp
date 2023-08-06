#include "ReadWriteFD.h"

#include <fcntl.h>

#define BUFSIZE 5120

ReadWriteFD::ReadWriteFD(const char * path) : ReadWriteFD(path, 0666) { }
ReadWriteFD::ReadWriteFD(const char * path, int oflag) : _buf{}, _fd{}, _path{}
{
#ifndef __SHAREDT_WIN__
    ::open(path, oflag);
#else
    _fd = CreateNamedPipeA(path, PIPE_ACCESS_DUPLEX,
                           PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                           PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 0, NULL);
#endif
    strcpy(_path, path);
}

char * ReadWriteFD::read()
{
    memset(_buf, 0, BUFSIZE);
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
    ::read(_fd, _buf, BUFSIZE);
#endif
    return _buf;
}

void ReadWriteFD::write(const char * buf) const
{
#ifdef __SHAREDT_WIN__
    WriteFile( _fd, buf, strlen(buf)+1, NULL, NULL);
#else
    ::write(_fd, buf, strlen(buf)+1);
#endif
}

