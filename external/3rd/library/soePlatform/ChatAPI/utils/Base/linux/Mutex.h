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

#ifndef	BASE_LINUX_MUTEX_H
#define	BASE_LINUX_MUTEX_H

#if !defined(_REENTRANT)
#   pragma message( "Excluding Base::CMutex - requires multi-threaded compile. (_REENTRANT)" ) 
#else


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
		    pthread_mutex_t mMutex;
            bool            mInitialized;
	};

    inline void CMutex::Lock(void)
    {
	    if (mInitialized)
            pthread_mutex_lock(&mMutex);
    }

    inline void CMutex::Unlock(void)
    {
        if (mInitialized)
            pthread_mutex_unlock(&mMutex);
    }

}
#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  #if defined(_MT)

#endif	// BASE_LINUX_MUTEX_H

