////////////////////////////////////////
//  Thread.cpp
//
//  Purpose:
// 	    1. Implementation of the CThread class.
//
//  Revisions:
// 	    07/10/2001  Created                                    
//

#if defined(_REENTRANT)


#include <pthread.h>
#include <time.h>
#include "Thread.h"

using namespace std;

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base
{

    void *threadProc(void *threadPtr)
	{
        CThread &thread = *((CThread*)threadPtr);
	    thread.mThreadActive = true;
        thread.ThreadProc();
        thread.mThreadActive = false;

        return 0;
	}

    CThread::CThread()
    {
        mThreadID = 0;
        mThreadActive = false;
        mThreadContinue = false;
    }

    CThread::~CThread()
    {
        StopThread();
    }

    void CThread::StartThread()
	{
	    mThreadContinue = true;
        pthread_create(&mThreadID,0,threadProc,this);
        while (!IsThreadActive())
            Base::sleep(1);
	}

    int32 CThread::StopThread(int timeout)
	{
        timeout += time(0);

        mThreadContinue = false;
        while (mThreadActive && time(0)<timeout)
            sleep(1);
        if (mThreadActive)
        {
            mThreadActive = false;
            return eSTOP_TIMEOUT;
        }
        return eSTOP_SUCCESS;
	}


////////////////////////////////////////////////////////////////////////////////


    CThreadPool::CMember::CMember(CThreadPool * parent) : 
        mParent(parent),
        mFunction(nullptr),
        mArgument(nullptr),
        mSemaphore()
    {
        StartThread();
    }

    CThreadPool::CMember::~CMember()
    {
    }

    void CThreadPool::CMember::Destroy()
    {
        mThreadContinue = false;
        mSemaphore.Signal();
    }

    bool CThreadPool::CMember::Execute(void( *function )( void * ), void * arg)
    {
        if (mFunction)
            return false;

        mArgument = arg;
        mFunction = function;
        mSemaphore.Signal();
        
        return true;
    }

    void CThreadPool::CMember::ThreadProc()
    {
        mParent->OnStartup(this);
        while (mThreadContinue)
        {
            mParent->OnIdle(this);
            mSemaphore.Wait(mParent->GetTimeOut()*1000);
            
            if (mFunction)
            {
                mFunction(mArgument);
                mArgument = nullptr;
                mFunction = nullptr;
            }
            else if (mParent->OnDestory(this))
                mThreadContinue = false;
        }
    }


////////////////////////////////////////////////////////////////////////////////

    
    CThreadPool::CThreadPool(uint32 maxThreads, uint32 minThreads, uint32 timeout) :
        mMutex(),
        mIdleMember(),
        mBusyMember(),
        mNullMember(),
        mThreadCount(0),
        mMaxThreads(maxThreads),
        mMinThreads(minThreads),
        mTimeOut(timeout)
    {
        if (mMaxThreads == 0) mMaxThreads = 1;
        if (mMinThreads == 0) mMinThreads = 1;
        if (mMinThreads > mMaxThreads) mMinThreads = mMaxThreads;

        for (uint32 i=0; i<mMinThreads; i++)
            new CMember(this);
    }

	CThreadPool::~CThreadPool()
    {
        set<CMember *>::iterator setIterator;

        ////////////////////////////////////////
        //  (1) Destory all busy member threads
        mMutex.Lock();
        setIterator = mBusyMember.begin();
        while (setIterator != mBusyMember.end())
            (*setIterator++)->Destroy();
        mMutex.Unlock();

        ////////////////////////////////////////
        //  (2) Destory all idle member threads
        while (mThreadCount)
        {
            mMutex.Lock();
            setIterator = mIdleMember.begin();
            while (setIterator != mIdleMember.end())
                (*setIterator++)->Destroy();
            mMutex.Unlock();

            sleep(1);
        }

        ////////////////////////////////////////
        //  (3) Delete the nullptr member threads
        mMutex.Lock();
        while (!mNullMember.empty())
        {
            delete mNullMember.front();
            mNullMember.pop_front();
        }
        mMutex.Unlock();
    }

    bool CThreadPool::Execute(void( *function )( void * ), void * arg)
    {
        mMutex.Lock();

        ////////////////////////////////////////
        //  (1) If no idle members, return false to indicate that no threads 
        //      were available.  If the thread count is below the max, create
        //      a new thread.
        if (mIdleMember.empty())
        {
            if (mThreadCount < mMaxThreads)
                new CMember(this);
            mMutex.Unlock();
            return false;
        }
        
        ////////////////////////////////////////
        //  (2) Delete any nullptr member threads.
        while (!mNullMember.empty())
        {
            delete mNullMember.front();
            mNullMember.pop_front();
        }

        ////////////////////////////////////////
        //  (3) Move the first idle thread to the busy set and signal the 
        //      thread to execute the specified function.
        CMember * member = *(mIdleMember.begin());
        mIdleMember.erase(member);
        mBusyMember.insert(member);
        member->Execute(function,arg);
        
        mMutex.Unlock();
        return true;
    }
            
    uint32 CThreadPool::GetTimeOut()
    {
        return mTimeOut;
    }

    void CThreadPool::OnStartup(CMember * member)
    {
        mMutex.Lock();
        
        mThreadCount++;
        mIdleMember.insert(member);

        mMutex.Unlock();
    }

    void CThreadPool::OnIdle(CMember * member)
    {
        mMutex.Lock();
        
        mBusyMember.erase(member);
        mIdleMember.insert(member);

        mMutex.Unlock();
    }

    bool CThreadPool::OnDestory(CMember * member)
    {
        set<CMember *>::iterator setIterator;

        mMutex.Lock();
        
        bool result = (setIterator = mIdleMember.find(member)) != mIdleMember.end();
        if (result)
        {
            mNullMember.push_back(member);
            mIdleMember.erase(setIterator);
            mThreadCount--;
        }

        mMutex.Unlock();

        return result;
    }


////////////////////////////////////////////////////////////////////////////////


}

#ifdef EXTERNAL_DISTRO
};
#endif
#endif  //  #if defined(_REENTRANT)
