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
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef	BASE_WIN32_PLATFORM_H
#define	BASE_WIN32_PLATFORM_H

#include <memory.h>
#include <winsock2.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include "Types.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    uint64 getTimer(void);
    uint64 getTimerFrequency(void);

    inline uint64 getTimer(void)
        {
        uint64 result;
        if (!QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&result)))
            result = 0;
        return result;
        }

    inline uint64 getTimerFrequency(void)
        {
        uint64 result;
        if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&result)))
            result = 0;
        return result;
        }

    inline void sleep(uint32 ms)
        {
	    Sleep(ms);
        }

    
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
};
#ifdef EXTERNAL_DISTRO
};
#endif


#endif	BASE_WIN32_PLATFORM_H