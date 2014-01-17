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
        mFunction(NULL),
        mArgument(NULL),
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

    bool CThreadPool::CMember::Execute(void(  *function )( void * ), void * arg)
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
        while (mThreadContinue)
        {
            mParent->OnIdle(this);
            mSemaphore.Wait(mParent->GetTimeOut()*1000);
            mParent->OnBusy(this);
            
            if (mFunction)
            {
                mFunction(mArgument);
                mArgument = NULL;
                mFunction = NULL;
            }
            else
                mThreadContinue = false;
        }

        mParent->OnDestory(this);
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

        if (mTimeOut < 60)
            mTimeOut = 60;

        for (uint32 i=0; i<mMinThreads; i++)
            new CMember(this);
    }

	CThreadPool::~CThreadPool()
    {
        set<CMember *>::iterator setIterator;

        mMutex.Lock();
        setIterator = mBusyMember.begin();
        while (setIterator != mBusyMember.end())
            (*setIterator++)->Destroy();
        mMutex.Unlock();

        while (mThreadCount)
        {
            mMutex.Lock();
            setIterator = mIdleMember.begin();
            while (setIterator != mIdleMember.end())
                (*setIterator++)->Destroy();
            mMutex.Unlock();

            sleep(1);
        }

        mMutex.Lock();
        while (!mNullMember.empty())
        {
            delete mNullMember.front();
            mNullMember.pop_front();
        }
        mMutex.Unlock();
    }

    bool CThreadPool::Execute(void( *function )( void * ), void * arg, uint32 poolGrowthSize )
    {
        mMutex.Lock();

        if (mIdleMember.empty())
        {
			if (mThreadCount < mMaxThreads)
			{
				if (!poolGrowthSize) 
					poolGrowthSize = mMinThreads;

				if ((mThreadCount + poolGrowthSize) > mMaxThreads)
					poolGrowthSize = mMaxThreads - mThreadCount;

				for (uint32 i(0); i < poolGrowthSize; i++)
					new CMember(this);
				mMutex.Unlock();
				time_t idleTimeout = time(0) + 5;
				while(mIdleMember.empty())
				{
					if (time(0) >= idleTimeout) 
					{
						return false;
					}
					Base::sleep(10);
				}
				mMutex.Lock();

			}
			else
			{
				mMutex.Unlock();
				return false;
			}
        }
        
        while (!mNullMember.empty())
        {
            delete mNullMember.front();
            mNullMember.pop_front();
        }

        CMember * member = *(mIdleMember.begin());
        mIdleMember.erase(member);
        member->Execute(function,arg);
        
        mMutex.Unlock();
        return true;
    }
            
    uint32 CThreadPool::GetTimeOut()
    {
        return mTimeOut;
    }

    void CThreadPool::OnIdle(CMember * member)
    {
        mMutex.Lock();
        
        set<CMember *>::iterator setIterator = mBusyMember.find(member);
        if (setIterator != mBusyMember.end())
            mBusyMember.erase(member);
        else
            mThreadCount++;
        mIdleMember.insert(member);

        mMutex.Unlock();
    }

    void CThreadPool::OnBusy(CMember * member)
    {
        mMutex.Lock();

        mBusyMember.insert(member);

        mMutex.Unlock();
    }

    void CThreadPool::OnDestory(CMember * member)
    {
        set<CMember *>::iterator setIterator;

        mMutex.Lock();
        
        setIterator = mBusyMember.find(member);
        if (setIterator != mBusyMember.end())
        {
            mNullMember.push_back(member);
            mBusyMember.erase(member);
            mThreadCount--;
        }

        mMutex.Unlock();
    }


////////////////////////////////////////////////////////////////////////////////


}


#endif  //  #if defined(_REENTRANT)
