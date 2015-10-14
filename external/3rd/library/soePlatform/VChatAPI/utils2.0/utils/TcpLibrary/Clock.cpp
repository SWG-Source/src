#include "Clock.h"

#include <time.h>

#ifdef WIN32
    #include <winsock.h>
#else //WIN32
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <sys/types.h>
#endif

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

Clock::Clock()
: m_lastStart(0),
  m_totalRunTime(0)
{
}

ClockStamp Clock::getCurTime()
{
#if defined(WIN32)
    static int sClockHigh = 0;
    static ClockStamp sClockLast = 0;

    int high = sClockHigh;
    DWORD low = GetTickCount();
    ClockStamp holdLast = sClockLast;        // this should be interlocked too
    ClockStamp ret = ((ClockStamp)high << 32) | low;

        // crazy trick to allow threading to work, by putting in a 1000 second fudge factor, we effective say
        // that it is ok to time-slice us at a bad point and we will still handle it, provided that our thread
        // gets processing time again within 1000 seconds
    if (ret < holdLast - 1000000)
    {
        sClockHigh = high + 1;
        ret = ((ClockStamp)high << 32) | low;
    }

    sClockLast = ret;        // this really should be interlocked to be totally safe since it is a 64 bit value, but I don't see a way to do that and am not sure it would mess up anything but the one call anyways

    return ret;
#else
    struct timeval tv;
    return (static_cast<ClockStamp>(tv.tv_sec) * 1000 + static_cast<ClockStamp>(tv.tv_usec / 1000));
#endif    
}

ClockStamp Clock::getElapsedSinceLastStart()
{
    if (m_lastStart == 0)
    {
        //hasn't been started
        return 0;
    }

    ClockStamp elapsed = getCurTime() - m_lastStart;

    if (elapsed > 2000000000)        // only time differences up to 23 days can be measured with this function
        elapsed = 2000000000;

    return elapsed;
}

void Clock::start()
{
    if (m_lastStart != 0)
    {
        //already started
        return;
    }

    //set last start to curtime
    m_lastStart = getCurTime();
}

void Clock::stop()
{
    if (m_lastStart == 0)
    {
        //need to start before stoping
        return;
    }

    m_totalRunTime += (unsigned)getElapsedSinceLastStart(); //rlsmith - explicit cast to prevent compiler warning
    m_lastStart = 0;
}


bool Clock::isDone(unsigned runTime)
{
    if (m_lastStart == 0)
    {
        //never started, so say no
        return false;
    }

    ClockStamp totalElapsed = getElapsedSinceLastStart() + m_totalRunTime;

    return (totalElapsed >= runTime);
}

void Clock::reset()
{
    m_lastStart = 0;
    m_totalRunTime = 0;
}



#ifdef EXTERNAL_DISTRO
};
#endif
