////////////////////////////////////////
//  Mutex.h
//
//  Purpose:
// 	    1. Declair the CMutex class that encapsulates the functionality of a 
//          mutually-exclusive device.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef	BASE_WIN32_MUTEX_H
#define	BASE_WIN32_MUTEX_H

#if defined (_MT)

#include "Platform.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    ////////////////////////////////////////
    //  Class:
    // 	    CMutex
    //
    //  Purpose:
    // 	    Encapsulates the functionality of a mutually-exclusive device.
    //      This class is valuable for protecting against race conditions
    //      within threaded applications.  The CMutex class can be used to
    //      only allow a single thread to run within a specified code 
    //      segment at a time.  
    //
    //  Public Methods:
    // 	    Lock() : Locks the mutex.  If the mutex is already locked, the 
    //          operating system will block the calling thread until another 
    //          thread has unlocked the mutex.
    //      Unlock() : Unlocks the mutex.
    //
    class CMutex
	{
	    public:
            CMutex();
            ~CMutex();

            void Lock();
		    void Unlock();
	    private:
		    CRITICAL_SECTION mCriticalSection;
	};

    inline void CMutex::Lock()
    {
        EnterCriticalSection(&mCriticalSection);
    }

    inline void CMutex::Unlock()
    {
        LeaveCriticalSection(&mCriticalSection);
    }

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif  //  #if defined(_MT)

#endif  //  BASE_WIN32_MUTEX_H