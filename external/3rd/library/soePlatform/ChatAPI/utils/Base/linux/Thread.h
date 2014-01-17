////////////////////////////////////////
//  Thread.h
//
//  Purpose:
// 	    1. Declair the CThread class that encapsulates threading functionality.  
//          This abstract base class in intended to be used to encapsulate 
//          individual tasks that require threading in derived classes.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#ifndef BASE_LINUX_THREAD_H
#define BASE_LINUX_THREAD_H

#if !defined(_REENTRANT)
#   pragma message( "Excluding Base::CThread - requires multi-threaded compile. (_REENTRANT)" ) 
#else


#pragma warning( disable : 4786)

#include <list>
#include <set>
#include "Platform.h"
#include "Mutex.h"
#include "Event.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base
{

    ////////////////////////////////////////
    //  Class:
    // 	    CThread
    //
    //  Purpose:
    // 	    Encapsulates threading functionality.  Creating classes derived
    //      from CThread provides an easy way to encapsulate tasks that require
    //      their own thread.  
    //
    //  Public Methods:
    // 	    StartThread() : Creates the low-level thread handle and begins executing
    //          the CThread::ThreadProc() function within the new thread.
    //      StopThread() : Signals the ThreadProc() function to stop executing using
    //          the mThreadContinue member variable, and waits for the ThreadProc()
    //          function to exit.  By default, the function will block for a maximum 
    //          of 5 seconds before exiting without the thread halting.
    //      IsThreadActive() : Returns true if the physical thread is still executing
    //          within the ThreadProc() function, otherwise it returns false.
    //      ThreadProc() : Pure-virtual function that will be executed when the StartThread()
    //          function is called.  Derived classes must implement this function.  The
    //          mThreadContinue member variable should be used internal the the ThreadProc()
    //          function to indicate whether it should continue executing or exit.
    //  Protected Attributes:
    //      mThreadContinue : Boolean value indicating to the ThreadProc() function
    //          whether to continue executing or to exit.  If mThreadContinue is true,
    //          ThreadProc() should continue, otherwise ThreadProc() should exit.  It
    //          left up to the derived class to implement a ThreadProc() function that
    //          uses the mThreadContinue member.
    //      
    //
    class CThread  
    {
        friend void *       threadProc(void *);

        public:
            enum { eSTOP_SUCCESS, eSTOP_TIMEOUT };
        public:
	        CThread();
	        virtual ~CThread();

            void            StartThread();
            int32           StopThread(int timeout=5);
            bool            IsThreadActive() { return mThreadActive; }

        protected:
            virtual void    ThreadProc() {}

	    protected:
		    bool            mThreadContinue;
        private:
		    pthread_t  		mThreadID;
            bool            mThreadActive;
    };


    class CThreadPool
    {
        private:
            class CMember : public CThread
            {
                public:
                    CMember(CThreadPool * parent);
                    virtual ~CMember();

                    bool Execute(void( *function )( void * ), void * arg);
                    void Destroy();
                
                protected:
                    virtual void    ThreadProc();

                private:
                    CThreadPool *   mParent;
                    void( * mFunction )( void * );
                    void *          mArgument;
                    CEvent          mSemaphore;
            };
            friend class CMember;

        public:
	        CThreadPool(uint32 maxThreads, uint32 minThreads=1, uint32 timeout=15*60);
	        virtual ~CThreadPool();

            virtual bool            Execute(void( *function )( void * ), void * arg, uint32 poolGrowthSize = 0);

        private:
            uint32          GetTimeOut();
            void            OnStartup(CMember * member);
            void            OnIdle(CMember * member);
            bool            OnDestory(CMember * member);

        private:
            CMutex                  mMutex;
            std::set<CMember *>     mIdleMember;
            std::set<CMember *>     mBusyMember;
            std::list<CMember *>    mNullMember;
            uint32                  mThreadCount;

            uint32                  mMaxThreads;
            uint32                  mMinThreads;
            uint32                  mTimeOut;
    };


}
#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  #if defined(_REENTRANT)

#endif  //  BASE_LINUX_THREAD_H
