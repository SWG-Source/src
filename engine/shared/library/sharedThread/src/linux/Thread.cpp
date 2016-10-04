// ======================================================================
//
// Thread.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedThread/Thread.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PerThreadData.h"
#include <cstdio>

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

pthread_key_t Thread::implindex;
Thread * Thread::mainThread;

void Thread::install()
{
	DEBUG_FATAL(mainThread, ("Thread::install: already installed"));
	ExitChain::add(remove, "Thread::remove");

	pthread_key_create(&implindex, 0);
	mainThread = new MainThread("Main");
	mainThread->attach();
}

void Thread::remove()
{
	DEBUG_FATAL(!mainThread, ("Thread::remove: not installed"));
	DEBUG_FATAL(mainThread->refcount > 1, ("Someone still holds the main thread"));
	delete mainThread;
	pthread_key_delete(implindex);

	mainThread = 0;
}

Thread::Thread(const std::string &i_name)
: refcount(1), name(i_name)
{
	doneFlag = false;
}

void Thread::start()
{
	pthread_create(&thread, 0, threadFunc, this);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
}

void Thread::attach()
{
	thread = pthread_self();
	pthread_setspecific(implindex, this);
}

Thread::~Thread()
{
	pthread_detach(thread);
}

void * Thread::threadFunc(void * i)
{
	Thread * impl = static_cast<Thread *>(i);
	pthread_setspecific(implindex,	impl);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
	Os::setThreadName(impl->thread, impl->name.c_str());
	PerThreadData::threadInstall(true);
	impl->run();
	PerThreadData::threadRemove();
	impl->kill();
	return 0;
}

void Thread::kill()
{
	int t = getCurrentThread()->thread;
	doneFlag = true;
	deref();
	if (this == getCurrentThread())
	{
		pthread_exit(0);
	}
	else
	{
		pthread_cancel(t);
	}
}

bool Thread::done()
{
	return doneFlag;
}

void Thread::wait()
{
	if (!doneFlag)
	{
		void * junk;
		pthread_join(thread, &junk);
	}
}

static inline int interpPriority(float t, int policy)
{
	return (int)(sched_get_priority_max(policy)*t + sched_get_priority_min(policy)*(1-t));
}

void Thread::setPriority(ePriority priority)
{
	static int policies[] =
	{
		SCHED_OTHER,
		SCHED_OTHER,
		SCHED_OTHER,
		SCHED_OTHER,
		SCHED_FIFO
	};
	static int priorities[] =
	{
		interpPriority(0.0, SCHED_OTHER),
		interpPriority(0.25, SCHED_OTHER),
		interpPriority(0.5, SCHED_OTHER),
		interpPriority(0.75, SCHED_OTHER),
		interpPriority(0.75, SCHED_FIFO)
	};
	sched_param p;
	p.sched_priority = priorities[priority];
	pthread_setschedparam(thread, policies[priority], &p);
}

