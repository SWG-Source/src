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

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base
{

    CEvent::CEvent() :
        mMutex(),
        mCond(),
        mThreadCount(0)
    {
        pthread_mutex_init(&mMutex, nullptr);
        pthread_cond_init(&mCond, nullptr);
    }

    CEvent::~CEvent()
    {
        pthread_cond_destroy(&mCond);
        pthread_mutex_destroy(&mMutex);
    }

    bool CEvent::Signal()
    {
        pthread_mutex_lock(&mMutex);
        if (mThreadCount == 0)
            mThreadCount = SIGNALED;
        pthread_cond_signal(&mCond);
        
        pthread_mutex_unlock(&mMutex);

        return true;
    }

    int32 CEvent::Wait(uint32 timeout)
    {
        int result;

        pthread_mutex_lock(&mMutex);
        if (mThreadCount == SIGNALED)
        {
            mThreadCount = 0;
            pthread_mutex_unlock(&mMutex);

            return eWAIT_SIGNAL;
        }

        if (!timeout)
        {
            mThreadCount++;
            result = pthread_cond_wait(&mCond, &mMutex);
            mThreadCount--;

            pthread_mutex_unlock(&mMutex);
        }
        else
        {
            struct timeval now;
            struct timespec abs_timeout;

            gettimeofday(&now, nullptr);
            abs_timeout.tv_sec  = now.tv_sec + timeout/1000;
            abs_timeout.tv_nsec = now.tv_usec * 1000 + (timeout%1000)*1000000;
            abs_timeout.tv_sec  += abs_timeout.tv_nsec / 1000000000;
            abs_timeout.tv_nsec %= 1000000000;

            mThreadCount++;
            result = pthread_cond_timedwait(&mCond, &mMutex, &abs_timeout);
            mThreadCount--;

            pthread_mutex_unlock(&mMutex);
        }

        if (result == 0 || result == EINTR)
            return eWAIT_SIGNAL;
        else if (result == ETIMEDOUT)
            return eWAIT_TIMEOUT;
        else
            return eWAIT_ERROR;
    }

}

#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  #if defined(_REENTRANT)
