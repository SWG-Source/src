////////////////////////////////////////
//  Platform.h
//
//  Purpose:
// 	    1. Include relevent system headers that are platform specific.
//      2. Declair global platform specific functionality.
//      3. Include primative type definitions
//
//  Global Functions:
// 	    getTimer() : Return the current high resolution clock count.
//      getTimerFrequency() : Return the frequency of the high resolution clock.
//      sleep() : Voluntarily relinquish timeslice of the calling thread for a 
//          specified number of milliseconds.
//      strlwr() : Alters the contents of a string, making it all lower-case.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef	BASE_LINUX_PLATFORM_H
#define	BASE_LINUX_PLATFORM_H

#include <errno.h>
#include <assert.h>
#include <sys/errno.h>
#include <pthread.h>
#include <resolv.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "Types.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base
{
    uint64 getTimer(void);
    uint64 getTimerFrequency(void);
    void sleep(uint32 ms);

    inline uint64 getTimer(void)
        {
	    uint64	t;
	    struct timeval	tv;
	    
	    gettimeofday(&tv, 0);
	    t = tv.tv_sec;
	    t = t * 1000000;
	    t += tv.tv_usec;
	    return t;
        }

    inline uint64 getTimerFrequency(void)
        {
	    uint64	f = 1000000;
	    return f;
        }

    inline void sleep(uint32 ms)
        {
	    usleep(static_cast<unsigned long>(ms * 1000));
        }

    void _strlwr(char * s);
    void _strupr(char * s);

    
    class CTimer
    {
        public:
            CTimer();

            void    Set(uint32 seconds);
            void    Signal();
            bool    Expired();

        private:
            uint32  mTimer;
    };

    inline void CTimer::Set(uint32 interval)
    {
        mTimer = (uint32)time(0) + interval;
    }

    inline void CTimer::Signal()
    {
        mTimer = 0;
    }

    inline bool CTimer::Expired()
    {
        return (mTimer <= (uint32)time(0));
    }


}
#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  BASE_LINUX_PLATFORM_H
