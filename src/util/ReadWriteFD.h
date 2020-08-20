#ifndef _READWRITEFD_H_
#define _READWRITEFD_H_

#include "Logger.h"
#include "Path.h"

#include <unistd.h>

#define MAX_BUF 512

class ReadWriteFD
{
  public:
    ReadWriteFD(const char * path);
    ReadWriteFD(const char * path, int oflag);
    ~ReadWriteFD() { ::close(_fd); }

    char * read();
    void   write(const char * buf);

    void open();
    void open(int flag);
    void close();

  private:
    ReadWriteFD();
    int  _fd;
    int  _flag;
    char _buf[MAX_BUF];
  protected:
    char _path[MAX_PATH];
};

#endif //_READWRITEFD_H_
