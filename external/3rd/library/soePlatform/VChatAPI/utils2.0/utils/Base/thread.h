#ifndef BASE__THREAD_H
#define BASE__THREAD_H

#if !defined(_MT) && !defined(_REENTRANT)
#error thread.h - requires multi-threaded compile
#endif

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/errno.h>
#include <pthread.h>
#endif

#ifdef NAMESPACE
namespace NAMESPACE
{
#endif

#ifdef WIN32
	typedef unsigned ThreadId_t;
	typedef CRITICAL_SECTION Mutex_t;
	typedef HANDLE Event_t;
	typedef void (*ThreadProc_t)(void *);
	void _threadProc(void *);
#else
	typedef pthread_t ThreadId_t;
	typedef pthread_mutex_t Mutex_t;
	typedef pthread_cond_t Event_t;
	typedef void* (*ThreadProc_t)(void *);
	void* _threadProc(void *);
#endif

ThreadId_t BeginThread(ThreadProc_t threadproc, void *arglist);
ThreadId_t GetThreadId();

class Mutex
{
	public:
        Mutex();
        ~Mutex();
        void Lock();
        bool TryLock();
		void Unlock();
	private:
		Mutex_t mMutex;
};

class ScopeLock
{
	public:
		ScopeLock(Mutex & mutex);
		~ScopeLock();
	private:
		Mutex & mMutex;
};

class SwitchableScopeLock
{
public:
	SwitchableScopeLock(Mutex & mutex, bool isOn);
	~SwitchableScopeLock();
private:
	Mutex & mMutex;
	bool mIsOn;
};

class Event
{
	public:
        Event(bool signaled = false);
        ~Event();
        bool Wait(unsigned timeoutMilliseconds = 0);
        void Signal();
	private:
#ifndef WIN32
		Mutex_t	mMutex;
		int		mThreadCount;
		pthread_cond_t mCond;
#endif
		Event_t mEvent;
};

class EventLock
{
	public:
        EventLock(bool locked = true);
        ~EventLock();
        bool Wait(unsigned timeoutMilliseconds = 0);
        void Lock();
        void Unlock();
		bool IsLocked();
	private:
#ifndef WIN32
		Mutex_t	mMutex;
		bool	mLocked;
		pthread_cond_t mCond;
#endif
		Event_t mEvent;
};

class RWLock
{
	public:
		RWLock();
		~RWLock();
		void ReadLock();
		void ReadUnlock();
		void WriteLock();
		void WriteUnlock();
	private:
		Mutex mWriteMutex;
		Mutex mReadMutex;
		unsigned short mReadCount;
		EventLock mReadLock;
		Event mReadEvent;
};

class Thread
{
#ifdef WIN32
	friend void _threadProc(void *);
#else
	friend void* _threadProc(void *);
#endif
	public:
		Thread();
		virtual ~Thread();
        void Start();
        bool Stop(unsigned timeoutSeconds=0);
		bool Active() const;
		bool GetId(ThreadId_t & threadId) const;
	protected:
        virtual void ThreadProc() = 0;
		bool Continue() const;  //	query for ThreadProc to continue or exit
		bool mContinue;
	private:
		ThreadId_t mThreadId;
		bool mActive;
};

#ifdef NAMESPACE
}
#endif

#endif
