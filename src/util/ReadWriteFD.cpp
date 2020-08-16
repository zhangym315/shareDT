#include "ReadWriteFD.h"
#include <fcntl.h>

ReadWriteFD::ReadWriteFD(const char * path)
{
    ReadWriteFD(path, 0666);
}

ReadWriteFD::ReadWriteFD(const char * path, int oflag)
{
    _fd = ::open(path, oflag);
    strcpy(_path, path);
    _flag = oflag;
}

char * ReadWriteFD::read()
{
    bzero(_buf, MAX_BUF);
    ::read(_fd, _buf, MAX_BUF);
    return _buf;
}

void ReadWriteFD::write(const char * buf)
{
    ::write(_fd, buf, strlen(buf)+1);
}

void ReadWriteFD::open()
{
    open(_flag);
}

void ReadWriteFD::open(int flag)
{
    _fd = ::open(_path, flag);
    _flag = flag;
}

void ReadWriteFD::close()
{
    ::close(_fd);
}
