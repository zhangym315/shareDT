#ifndef __PID_H__
#define __PID_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _WIN32

#include "WinSock2.h"
#include "Windows.h"
typedef DWORD portable_pid_t;
typedef DWORD portable_tid_t;
#define get_portable_pid()		(GetCurrentProcessId())
#define get_portable_tid()		(GetCurrentThreadId())

#else /* !_WIN32 */

#  include <sys/types.h>
#  include <unistd.h>

typedef pid_t portable_pid_t;
#  define get_portable_pid()		(getpid())

/* Add TID support */
#ifdef __APPLE__

#  include <sys/syscall.h>
typedef uint64_t portable_tid_t;

#elif defined(__linux__)

#  include <sys/syscall.h>
typedef pid_t portable_tid_t;
#  define get_portable_tid()		((portable_tid_t)syscall(SYS_gettid))

#elif defined(__sun__)

#include <thread.h>
typedef thread_t portable_tid_t;
#  define get_portable_tid()		(thr_self())

#elif defined(_AIX)

#include <sys/thread.h>
typedef tid_t portable_tid_t;
#  define get_portable_tid()		(thread_self())

#elif defined(__FreeBSD__) || defined(__OpenBSD__)

typedef long portable_tid_t;

#else
#  error "This platform does not have get_portable_tid() implementation!"
#endif

#ifndef get_portable_tid
extern portable_tid_t get_portable_tid(void);
#endif /* !get_portable_tid */

#endif /* _WIN32 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__PID_H__ */
