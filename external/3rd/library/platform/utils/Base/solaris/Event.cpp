////////////////////////////////////////
//  Event.cpp
//
//  Purpose:
// 	    1. Implementation of the CEvent class.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#if defined(_REENTRANT)


#include "Event.h"

namespace Base
{

    CEvent::CEvent()
    {
        mWaiting = 0;
        mSignaled = false; 
        mInitialized = false;
        if (pthread_cond_init( &mCond, NULL ) != 0)
        {
            return;
        }

        if (pthread_mutex_init( &mMutex, NULL ) != 0)
        {
            pthread_cond_destroy( &mCond );
            return;
        }
        mInitialized = true;
    }

    CEvent::~CEvent()
    {
        if (mInitialized)
        {
            pthread_cond_destroy(&mCond);
            pthread_mutex_destroy(&mMutex);
        }
    }
    
    int32 CEvent::Wait(uint32 timeout)
    {
        if (!mInitialized)
            return CEvent::eWAIT_ERROR;

        int result;
        pthread_mutex_lock(&mMutex);
	
	if (mSignaled)
	{
	    mSignaled = false;
	}
        else if (!timeout)
        {
            mWaiting++;
            result = pthread_cond_wait(&mCond, &mMutex);
            mWaiting--;
        }
        else
        {
            struct timespec wake_time;

            clock_gettime( CLOCK_REALTIME, &wake_time );
            wake_time.tv_sec  += timeout/1000;
            wake_time.tv_nsec += (timeout%1000)*1000000;

            // normalize new time
            wake_time.tv_sec  += wake_time.tv_nsec / 1000000000;
            wake_time.tv_nsec %= 1000000000;
            
            mWaiting++;
            result = pthread_cond_timedwait( &mCond, &mMutex, &wake_time );
            mWaiting--;
        }
	pthread_mutex_unlock(&mMutex);

        if (result == 0)
            return CEvent::eWAIT_SIGNAL;
        else if (result == ETIMEDOUT)
            return CEvent::eWAIT_TIMEOUT;
        else
            return CEvent::eWAIT_ERROR;
    }

}


#endif  //  #if defined(_REENTRANT)
