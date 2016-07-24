////////////////////////////////////////
//  Event.h
//
//  Purpose:
// 	    1. Declair the CEvent class that encapsulates the functionality of a 
//          single-locking semaphore.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef BASE_WIN32_EVENT_H
#define BASE_WIN32_EVENT_H

#if defined(_MT)



#include "Platform.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    ////////////////////////////////////////
    //  Class:
    // 	    CEvent
    //
    //  Purpose:
    // 	    Encapsulates the functionality of a singal-locking semaphore.
    //      This class is valuable for thread syncronization when a thead's
    //      execution needs to be dependent upon another thread.  
    //
    //  Public Methods:
    // 	    Signal() : Signals a thread that has called Wait() so that it can 
    //          continue execution.  This function returns true if the waiting 
    //          thread was signalled successfully, otherwise false is returned.
    //      Wait() : Halts the calling thread's execution indefinately until
    //          a Singal() call is made by an external thread.  If the thread is 
    //          successfully signalled, the function returns eWAIT_SIGNAL.  If
    //          timeout period expires without a signal, eWAIT_TIMEOUT is returned.
    //          If the function fails, eWAIT_ERROR is returned.
    //
    class CEvent  
        {
        public:
	        CEvent();
	        virtual ~CEvent();

            bool    Signal();
            int32   Wait(uint32 timeout = 0);

        public:
            enum { eWAIT_ERROR, eWAIT_SIGNAL, eWAIT_TIMEOUT };
        private:
            HANDLE  mEvent;
        };

    inline bool CEvent::Signal()
        {
        if (mEvent)
            return SetEvent(mEvent)!=0;
    
        return false;
        }

    inline int32 CEvent::Wait(uint32 timeout)
        {
        if (!mEvent)
            return CEvent::eWAIT_ERROR;
        
        DWORD result = WaitForSingleObjectEx(mEvent, timeout ? timeout : INFINITE, FALSE);
        if (result == WAIT_OBJECT_0)
            return CEvent::eWAIT_SIGNAL;
        else if (result == WAIT_TIMEOUT)
            return CEvent::eWAIT_TIMEOUT;
        else
            return CEvent::eWAIT_ERROR;
        }

};
#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  #if defined(_MT)


#endif  //  BASE_WIN32_EVENT_H
