#include "time_utils.h"

#include <sys/time.h>

long CTimeUtils::GetCurTimeS()
{
    struct timeval  tv;
    struct timezone tz;

    tz.tz_minuteswest = 0;
    tz.tz_dsttime = 0;
    gettimeofday(&tv, &tz);
    return  tv.tv_sec;
}

long CTimeUtils::GetCurTimeMs()
{
    return GetCurTimeUs() / 1000;
}

long CTimeUtils::GetCurTimeUs()
{
    struct timeval  tv;
    struct timezone tz;
    long cur_time_us = 0;

    tz.tz_minuteswest = 0;
    tz.tz_dsttime = 0;
    gettimeofday(&tv, &tz);
    cur_time_us = tv.tv_sec * 1000000 + tv.tv_usec;
    return cur_time_us;
}

