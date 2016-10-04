#ifdef WIN32
#pragma warning (disable: 4514 4786)
#endif

#if !defined(_MT) && !defined(_REENTRANT)
#pragma message ("excluding thread.cpp - requires multi-threaded compile")
#else

#include "timer.h"
#include "thread.h"
#include <time.h>
#include <assert.h>
#ifdef WIN32
#include <process.h>
#endif

#ifdef NAMESPACE
namespace NAMESPACE
{
#endif

ThreadId_t BeginThread(ThreadProc_t threadproc, void *arglist)
{
	ThreadId_t threadId;
#ifdef WIN32
    threadId = _beginthread(*threadproc,0,arglist);
#else
    pthread_create(&threadId,0,threadproc,arglist);
#endif
	return threadId;
}

ThreadId_t GetThreadId()
{
	unsigned threadId;
#ifdef WIN32
	threadId = GetCurrentThreadId();
#else
    threadId = pthread_self();
#endif
	return threadId;
}

Mutex::Mutex()
{
#ifdef WIN32
    InitializeCriticalSection(&mMutex);
#else
	pthread_mutex_init(&mMutex, 0);
#endif
}

Mutex::~Mutex()
{
#ifdef WIN32
    DeleteCriticalSection(&mMutex);
#else
    pthread_mutex_destroy(&mMutex);
#endif
}

void Mutex::Lock()
{
#ifdef WIN32
	EnterCriticalSection(&mMutex);
#else
    pthread_mutex_lock(&mMutex);
#endif
}

bool Mutex::TryLock()
{
#ifdef WIN32
	Lock();
    return false;
#else
    return pthread_mutex_trylock(&mMutex) != EBUSY;
#endif
}

void Mutex::Unlock()
{
#ifdef WIN32
    LeaveCriticalSection(&mMutex);
#else
    pthread_mutex_unlock(&mMutex);
#endif
}

ScopeLock::ScopeLock(Mutex & mutex) :
mMutex(mutex)
{
	mMutex.Lock();
}

ScopeLock::~ScopeLock()
{
	mMutex.Unlock();
}

SwitchableScopeLock::SwitchableScopeLock(Mutex & mutex, bool isOn) :
	mMutex(mutex),
	mIsOn(isOn)
{
	if(mIsOn) { mMutex.Lock(); }
}

SwitchableScopeLock::~SwitchableScopeLock()
{
	if(mIsOn) { mMutex.Unlock(); }
}

const int EVENT_SIGNALED = -1;

Event::Event(bool signaled) :
#ifndef WIN32
	mMutex(),
	mThreadCount(signaled ? EVENT_SIGNALED : 0),
#endif
	mEvent()
{
#ifdef WIN32
	mEvent = CreateEvent(nullptr, FALSE, signaled ? TRUE : FALSE, nullptr);
#else
    pthread_mutex_init(&mMutex, nullptr);
    pthread_cond_init(&mCond, nullptr);
#endif
}

Event::~Event()
{
#ifdef WIN32
    CloseHandle(mEvent);
#else
    pthread_cond_destroy(&mCond);
    pthread_mutex_destroy(&mMutex);
#endif
}

//	return: true if signaled, false if timeout/error
bool Event::Wait(unsigned timeoutMilliseconds)
{
#ifdef WIN32
    return WaitForSingleObjectEx(mEvent, timeoutMilliseconds ? timeoutMilliseconds : INFINITE, FALSE) == WAIT_OBJECT_0;
#else
    pthread_mutex_lock(&mMutex);
    if (mThreadCount == EVENT_SIGNALED)
    {
        mThreadCount = 0;
        pthread_mutex_unlock(&mMutex);
        return true;	// signaled
    }
    if (!timeoutMilliseconds)
    {
        mThreadCount++;
        pthread_cond_wait(&mCond, &mMutex);
        mThreadCount--;
        pthread_mutex_unlock(&mMutex);
		return true;	// signaled
    }
    else
    {
        struct timeval now;
        struct timespec abs_timeout;
        gettimeofday(&now, nullptr);
        abs_timeout.tv_sec  = now.tv_sec + timeoutMilliseconds/1000;
        abs_timeout.tv_nsec = now.tv_usec * 1000 + (timeoutMilliseconds%1000)*1000000;
        abs_timeout.tv_sec  += abs_timeout.tv_nsec / 1000000000;
        abs_timeout.tv_nsec %= 1000000000;
        mThreadCount++;
        int waitResult = pthread_cond_timedwait(&mCond, &mMutex, &abs_timeout);
        mThreadCount--;
        pthread_mutex_unlock(&mMutex);
        return (waitResult == 0 || waitResult == EINTR);
    }
#endif
}

void Event::Signal()
{
#ifdef WIN32
    SetEvent(mEvent);
#else
    pthread_mutex_lock(&mMutex);
    if (mThreadCount == 0)
        mThreadCount = EVENT_SIGNALED;
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);
#endif
}


EventLock::EventLock(bool locked) :
#ifndef WIN32
	mMutex(),
	mLocked(locked),
#endif
	mEvent()
{
#ifdef WIN32
    mEvent = CreateEvent(nullptr, TRUE, locked ? FALSE : TRUE, nullptr);
#else
    pthread_mutex_init(&mMutex, nullptr);
    pthread_cond_init(&mCond, nullptr);
#endif
}

EventLock::~EventLock()
{
#ifdef WIN32
    CloseHandle(mEvent);
#else
    pthread_cond_destroy(&mCond);
    pthread_mutex_destroy(&mMutex);
#endif
}

bool EventLock::IsLocked()
{
#ifdef WIN32
	return WaitForSingleObjectEx(mEvent, 0, FALSE) != WAIT_OBJECT_0;
#else
	return mLocked;
#endif
}

bool EventLock::Wait(unsigned timeoutMilliseconds)
{
#ifdef WIN32
    return WaitForSingleObjectEx(mEvent, timeoutMilliseconds ? timeoutMilliseconds : INFINITE, FALSE) == WAIT_OBJECT_0;
#else
    pthread_mutex_lock(&mMutex);
	if (mLocked)
	{
		if (!timeoutMilliseconds)
		{
			pthread_cond_wait(&mCond, &mMutex);
			pthread_mutex_unlock(&mMutex);

			return true;	// signaled
		}
		else
		{
			struct timeval now;
			struct timespec abs_timeout;
			gettimeofday(&now, nullptr);
			abs_timeout.tv_sec  = now.tv_sec + timeoutMilliseconds/1000;
			abs_timeout.tv_nsec = now.tv_usec * 1000 + (timeoutMilliseconds%1000)*1000000;
			abs_timeout.tv_sec  += abs_timeout.tv_nsec / 1000000000;
			abs_timeout.tv_nsec %= 1000000000;
			int waitResult = pthread_cond_timedwait(&mCond, &mMutex, &abs_timeout);
			pthread_mutex_unlock(&mMutex);

			return (waitResult == 0 || waitResult == EINTR);
		}
	} else {
		pthread_mutex_unlock(&mMutex);
	}

	return true;
#endif
}

void EventLock::Lock()
{
#ifdef WIN32
    ResetEvent(mEvent);
#else
    pthread_mutex_lock(&mMutex);
	mLocked = true;
    pthread_mutex_unlock(&mMutex);
#endif
}

void EventLock::Unlock()
{
#ifdef WIN32
    SetEvent(mEvent);
#else
    pthread_mutex_lock(&mMutex);
	mLocked = false;
    pthread_cond_broadcast(&mCond);
    pthread_mutex_unlock(&mMutex);
#endif
}


RWLock::RWLock() :
	mWriteMutex(),
	mReadMutex(),
	mReadCount(0),
	mReadLock(),
	mReadEvent()
{
}

RWLock::~RWLock()
{
}

void RWLock::ReadLock()
{
	mReadMutex.Lock();
	unsigned short readCount = mReadCount++;
	mReadMutex.Unlock();
	if (readCount == 0)
	{
		mReadEvent.Wait();
		mReadLock.Unlock();
	}
	mReadLock.Wait();
}

void RWLock::ReadUnlock()
{
	assert(mReadCount);
	mReadMutex.Lock();
	unsigned short readCount = --mReadCount;
	mReadMutex.Unlock();
	if (readCount == 0)
	{
		mReadLock.Lock();
		mReadEvent.Signal();
	}

}

void RWLock::WriteLock()
{
	mWriteMutex.Lock();
	mReadEvent.Wait();
}

void RWLock::WriteUnlock()
{
	mReadEvent.Signal();
	mWriteMutex.Unlock();
}


Thread::Thread() :
	mThreadId(0),
	mActive(false),
	mContinue(false)
{
}

Thread::~Thread()
{
	Stop();
}

#ifdef WIN32
void _threadProc(void *threadPtr)
{
    Thread & thread = *((Thread*)threadPtr);
	thread.mActive = true;
    thread.ThreadProc();
    thread.mActive = false;
}
#else
void* _threadProc(void *threadPtr)
{
    Thread & thread = *((Thread*)threadPtr);
	thread.mActive = true;
    thread.ThreadProc();
    thread.mActive = false;
	return 0;
}
#endif

void Thread::Start()
{
	mContinue = true;
    mThreadId = BeginThread(_threadProc,this);
}

//	return true if thread became inactive before the timeout
bool Thread::Stop(unsigned timeoutSeconds)
{
    mContinue = false;
    timeoutSeconds += time(0);
    while (mActive && (unsigned)time(0)<timeoutSeconds)
		soe::Sleep(100);
    return (!mActive);
}

bool Thread::Active() const
{
	return mActive;
}

bool Thread::GetId(ThreadId_t & threadId) const
{
	/*
	if (!mActive)
		return false;
	*/
	threadId = mThreadId;
	return true;
}

bool Thread::Continue() const
{
	return mContinue;
}


#ifdef NAMESPACE
}
#endif

#endif
