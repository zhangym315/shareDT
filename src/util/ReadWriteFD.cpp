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
    open(O_RDONLY);
    bzero(_buf, MAX_BUF);
    ::read(_fd, _buf, MAX_BUF);
    close();
    return _buf;
}

void ReadWriteFD::write(const char * buf)
{
    open(O_WRONLY);
    ::write(_fd, buf, strlen(buf)+1);
    close();
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
