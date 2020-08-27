#ifndef _READWRITEFD_H_
#define _READWRITEFD_H_

#include "Logger.h"
#include "Path.h"
#include "CrossPlatform.h"
#include "TypeDef.h"

#ifdef __SHAREDT_WIN__
#include <windows.h>
#endif

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
    ~ReadWriteFD()
    {
        OS_CLOSE(_fd);
    }

    char * read();
    void   write(const char * buf);

    void open();
    void open(int flag);
    void close();

  private:
    ReadWriteFD();
#ifdef __SHAREDT_WIN__
    HANDLE _fd;
#else
    int  _fd;
#endif
    int  _flag;
    char _buf[BUFSIZE];
  protected:
    char _path[MAX_PATH];
};

#endif //_READWRITEFD_H_
