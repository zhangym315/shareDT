#include "TimeUtil.h"
#include <stdio.h>
#include <time.h>

#ifndef __SHAREDT_WIN__
#include <sys/time.h>
#else

#include <windows.h>

static int gettimeofdayLocal( struct timeval *tv)
{
    time_t rawtime;

    time(&rawtime);
    tv->tv_sec = (long)rawtime;

    LARGE_INTEGER tickPerSecond;
    LARGE_INTEGER tick; // a point in time

    QueryPerformanceFrequency(&tickPerSecond);
    QueryPerformanceCounter(&tick);

    tv->tv_usec = (tick.QuadPart % tickPerSecond.QuadPart);

    return 0;
}

#endif

char * get_current_time_string()
{
    static char result[32];
    struct timeval currentTime;
    struct tm * timeptr;
#ifdef __SHAREDT_WIN__
    gettimeofdayLocal(&currentTime);
#else
    gettimeofday(&currentTime, NULL);
#endif
    time_t time = (time_t)currentTime.tv_sec;

    timeptr = localtime (&time);


    snprintf(result, 32, "%.2d-%.2d-%d %.2d:%.2d:%.2d.%.3d",
            timeptr->tm_mon+1,
            timeptr->tm_mday,
            1900 + timeptr->tm_year,
            timeptr->tm_hour,
            timeptr->tm_min,
            timeptr->tm_sec,
            (int) currentTime.tv_usec/1000);

    return result;
}