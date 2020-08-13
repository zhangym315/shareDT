#ifndef _UTIL_TYPEDEF_H_
#define _UTIL_TYPEDEF_H_

#include <thread>

#ifdef _WIN32
typedef int pid_t;
#endif

#ifdef _WIN32
#  define THREAD_STARTFUNC_RETURN_DECL unsigned __stdcall
#include <string>
#else // ! def _WIN32
#  define THREAD_STARTFUNC_RETURN_DECL void*
#endif // ! def _WIN32

#if defined(WINDOWS) || defined(WIN32)
#if defined(SC_LITE_DLL)
#define DELC_EXPORT __declspec(dllexport)
#else
#define DELC_EXPORT
#endif
#else
#define DELC_EXPORT
#endif

typedef unsigned char   uchar;
typedef std::string     String;
typedef pid_t           Pid;

#define ARRAY_SIZE(arr)	(sizeof(arr) / sizeof(0[arr]))

#ifndef OS_ALLOCATE
#  define STACK_ALLOCATE_DOESNT_CONSTRUCT
#  ifdef __SHAREDT_WIN__
#    include <malloc.h>
#    define OS_ALLOCATE(_type, _name, _nmemb)		\
	_type *_name = (_type *) _alloca((_nmemb) * sizeof(_type))
#  else /* !__SHAREDT_WIN__ */
#    ifdef HAVE_ALLOCA_H
#      include <alloca.h>
#    else // !HAVE_ALLOCA_H
#      include <stdlib.h>
#    endif // HAVE_ALLOCA_H
#    define OS_ALLOCATE(_type, _name, _nmemb)		\
	_type *_name = (_type *) alloca((_nmemb) * sizeof(_type))
#  endif /* __SHAREDT_WIN__ */
#endif /* !OS_ALLOCATE */

#define chars_equal(p1, p2)	(0 == strcmp((p1), (p2)))

#endif