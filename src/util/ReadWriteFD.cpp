#include "ReadWriteFD.h"
#include <fcntl.h>

ReadWriteFD::ReadWriteFD(const char * path)
{
    ReadWriteFD(path, 0666);
}

ReadWriteFD::ReadWriteFD(const char * path, int oflag)
{

    OS_OPEN(path, oflag);
    strcpy(_path, path);
    _flag = oflag;
}

char * ReadWriteFD::read()
{
    open(O_RDONLY);
    bzero(_buf, MAX_BUF);
    OS_READ(_fd, _buf, MAX_BUF);

    close();
    return _buf;
}

void ReadWriteFD::write(const char * buf)
{
    open(O_WRONLY);
    OS_WRITE(_fd, buf, strlen(buf)+1);
    close();
}

void ReadWriteFD::open()
{
    open(_flag);
}

void ReadWriteFD::open(int flag)
{
    _fd = OS_OPEN(_path, flag);
    _flag = flag;
}

void ReadWriteFD::close()
{
    OS_CLOSE(_fd);
}
