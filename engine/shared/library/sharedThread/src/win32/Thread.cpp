// ======================================================================
//
// Thread.cpp
// Acy Stapp
//
// Copyright6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedThread/FirstSharedThread.h"
#include "sharedThread/Thread.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedSynchronization/Mutex.h"
#include "sharedSynchronization/RecursiveMutex.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PerThreadData.h"
#include <process.h>

#include <string>

class MainThread: public Thread
{
public:
	MainThread(const std::string &name): Thread(name) {}
protected:
	void run() {}
private:
	MainThread(const MainThread &o);
	MainThread &operator =(const MainThread &o);
};

int Thread::implindex;
Thread * Thread::mainThread;

void Thread::install()
{
	DEBUG_FATAL(mainThread, ("Thread::install: already installed"));
	
	implindex = TlsAlloc();
	mainThread = new MainThread("Main");
	mainThread->attach();

	ExitChain::add(remove, "Thread::remove");

	Mutex::install();
	RecursiveMutex::install();

}

void Thread::remove()
{
	DEBUG_FATAL(!mainThread, ("Thread::remove: not installed"));
	DEBUG_FATAL(mainThread->refcount > 1, ("Someone still holds the main thread"));
	delete mainThread; // can't kill because it would terminate the app
	TlsFree(implindex);

	Mutex::remove();
	RecursiveMutex::remove();

	mainThread = 0; 
}

Thread::Thread()
: refcount(1), name(new std::string)
{
}

Thread::Thread(const std::string &i_name)
: refcount(1), name(new std::string(i_name))
{
}

void Thread::start()
{
	handle = (HANDLE) _beginthreadex(0, 0, threadFunc, this, 0, &id);
}

void Thread::attach()
{
	id = GetCurrentThreadId();
	HANDLE process = GetCurrentProcess();
	DuplicateHandle(process, GetCurrentThread(), process, &handle, 0, FALSE, DUPLICATE_SAME_ACCESS);

	TlsSetValue(implindex, this);
	Os::setThreadName(id, name->c_str());
}

Thread::~Thread()
{
	CloseHandle(handle);
	delete name;
}

unsigned int Thread::threadFunc(void * i)
{
	Thread * impl = static_cast<Thread *>(i);
	TlsSetValue(implindex, impl);
	Os::setThreadName(impl->id, impl->name->c_str());
	PerThreadData::threadInstall(true);
	impl->run();
	PerThreadData::threadRemove();
	impl->kill();
	return 0;
}

void Thread::kill()
{
	void * h = getCurrentThread()->handle;
	deref();
	if (this == getCurrentThread())
	{
		_endthreadex(1);
	}
	else
	{
		TerminateThread(h, 1);
	}
}

void Thread::wait()
{
	WaitForSingleObject(handle, INFINITE);
}

bool Thread::done()
{
	return WaitForSingleObject(handle, 0) == WAIT_OBJECT_0; // WAIT_TIMEOUT if the thread is still active
}

void Thread::setName(const std::string &i_name)
{
	*name = i_name;
	Os::setThreadName(id, name->c_str());
}

void Thread::setPriority(ePriority priority)
{
	static int priorities[] =
	{
		THREAD_PRIORITY_IDLE,
		THREAD_PRIORITY_LOWEST,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_TIME_CRITICAL
	};
	::SetThreadPriority(handle, priorities[priority]);
}

void Thread::suspend()
{
	::SuspendThread(handle);
}

void Thread::resume()
{
	::ResumeThread(handle);
}

const std::string &Thread::getName() const
{
	return *name;
}

// Make sure the header-only files compile :)

#if 0

#include "sharedSynchronization/CountingSemaphore.h"
#include "sharedSynchronization/BlockingPointer.h"
#include "sharedSynchronization/BlockingQueue.h"
#include "sharedSynchronization/WriteOnce.h"

Mutex t;
BlockingQueue<int> bqint(t, 0, 0);
BlockingPointer<int> bpint(t, 0, 0);
WriteOnce<int> woint;

#endif