#ifndef _PORTABLE_PLATFORM_H_
#define _PORTABLE_PLATFORM_H_

#define OSPATH_SEP_CHAR_UNIX            '/'
#define OSPATH_SEP_STR_UNIX             "/"
#define OSPATH_SEP_CHAR_WIN             '\\'
#define OSPATH_SEP_STR_WIN              "\\"

#ifdef _WIN32
/* character OS uses to divide elements of a filename */
#  define OSPATH_SEP_CHAR		OSPATH_SEP_CHAR_WIN
#  define OSPATH_SEP_STR		OSPATH_SEP_STR_WIN
   /* character OS uses to separate elements in a %PATH%-style list */
#  define OSPATH_DIVIDE_CHAR		';'
#  define OSPATH_DIVIDE_STR		";"
#  ifndef PATH_MAX
#    include "WinSock2.h"	/* needed before Windows.h (it can't be included after) */
#    include "Windows.h"	/* needed for MAX_PATH */
#    define PATH_MAX			(MAX_PATH)
#  endif /* !PATH_MAX */
   /* in win, both / and \ seem to be valid separators for access() etc. */
#  define OSPATH_IS_SEP(x)              (   OSPATH_SEP_CHAR_WIN  == (x) \
					 || OSPATH_SEP_CHAR_UNIX == (x))
#else /* !_WIN32 */
#  include <limits.h>	/* needed for PATH_MAX on Solaris */
#  define OSPATH_SEP_CHAR		OSPATH_SEP_CHAR_UNIX
#  define OSPATH_SEP_STR		OSPATH_SEP_STR_UNIX
#  define OSPATH_DIVIDE_CHAR		':'
#  define OSPATH_DIVIDE_STR		":"
#  define OSPATH_IS_SEP(x)              (OSPATH_SEP_CHAR == (x))
#endif /* _WIN32 */

#endif //_PORTABLE_PLATFORM_H_
