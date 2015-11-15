#ifndef	SOE__TIMER_H
#define	SOE__TIMER_H


#ifdef WIN32

#include <winsock2.h>

#elif linux

#include <sys/time.h>
#include <unistd.h>

#endif

#include "types.h"


namespace soe
{


#ifdef WIN32

    
	inline uint64 GetTimer(void)
    {
        uint64 result;
        if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&result)))
            result = 0;
        return result;
    }

    inline int64 GetTimerFrequency(void)
    {
        int64 result;
        if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&result)))
            result = 0;
        return result;
    }

    inline void Sleep(uint32 ms)
    {
		::Sleep(ms);
    }


#elif linux

    inline uint64 GetTimer(void)
    {
		uint64 t;
		struct timeval	tv;
		
		gettimeofday(&tv, 0);
		t = tv.tv_sec;
		t = t * 1000000;
		t += tv.tv_usec;
		return t;
    }

    inline int64 GetTimerFrequency(void)
    {
		int64 f = 1000000;
		return f;
    }

    inline void Sleep(uint32 ms)
    {
		usleep(static_cast<unsigned long>(ms * 1000));
    }


#endif


    inline double GetTimerLatency(soe::uint64 startTime, soe::uint64 finishTime=0)
    {
        soe::int64 requestAge;
        soe::uint64 finish = (finishTime ? finishTime : soe::GetTimer());
        if (finish < startTime)
            requestAge = (0 - 1) - (startTime - finish) + 1;
        else
            requestAge = finish - startTime;
        return (double)requestAge/soe::GetTimerFrequency();
    }


}


#endif

