#ifndef _READWRITEFD_H_
#define _READWRITEFD_H_

#ifdef __SHAREDT_WIN__
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Logger.h"
#include "Path.h"
#include "TypeDef.h"

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
#ifdef __SHAREDT_WIN__
    ReadWriteFD(HANDLE h) : _fd(h) { }
#endif
    explicit ReadWriteFD(const char * path);
    explicit ReadWriteFD(const char * path, int oflag);
    ~ReadWriteFD()
    {
#ifdef __SHAREDT_WIN__
        CloseHandle(_fd);
#else
        ::close(_fd);
#endif
    }

    char * read();
    void   write(const char * buf) const;

private:
    ReadWriteFD() = default;

#ifdef __SHAREDT_WIN__
    HANDLE _fd;
#else
    int  _fd;
#endif
    char _buf[BUFSIZE];
    char _path[MAX_PATH];
};

#endif //_READWRITEFD_H_
