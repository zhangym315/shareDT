#include "Pid.h"

#ifdef __APPLE__

#  include <pthread.h>

portable_tid_t get_portable_tid(void)
{
    portable_tid_t tid;
    if (pthread_threadid_np(NULL, &tid) != 0)
        tid = (portable_tid_t) get_portable_pid(); /* Shouldn't be possible */
    return tid;
}

#endif /* __APPLE__ */

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sys/thr.h>

portable_tid_t get_portable_tid(void)
{
    portable_tid_t tid = 0;
    if (thr_self(&tid) != 0) {
        // the only way this could happen is OOM - should we return 0 instead ?
        return (portable_tid_t)get_portable_pid();
    }
    return tid;
}
#endif
