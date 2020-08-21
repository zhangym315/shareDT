#ifndef _READWRITEFD_H_
#define _READWRITEFD_H_

#include "Logger.h"
#include "Path.h"

#include <unistd.h>

#define MAX_BUF 512

/*
 * This is class supposed to read and write to pipe between
 * process communication.
 * So every time read and write, we needs
 * to open, and close after read and write to pipe, in order
 * to make sure the message can be sent or received successfully.
 */
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
