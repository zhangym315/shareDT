#ifndef SHAREDT_CROSSPLATFORM_H
#define SHAREDT_CROSSPLATFORM_H

#ifdef __SHAREDT_WIN__
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#endif

#ifdef __SHAREDT_WIN__
#include <io.h>

#define OS_OPEN _open
#define OS_READ _read
#define OS_WRITE _write
#define OS_CLOSE _close
#else
#include <unistd.h>
#define OS_OPEN ::open
#define OS_READ ::read
#define OS_WRITE ::write
#define OS_CLOSE ::close
#endif

#endif //SHAREDT_CROSSPLATFORM_H
