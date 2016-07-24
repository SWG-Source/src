// ======================================================================
//
// Thread.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Thread_h
#define INCLUDED_Thread_h

#include "sharedSynchronization/InterlockedInteger.h"

template <class T> class TypedThreadHandle;

class Thread
{
	// Visual studio fails to compile this valid code
	// template <class T> friend TypedThreadHandle;
public:
	static void install();
	static void remove();

	Thread();
	Thread(const std::string &i_name);

	void kill();
	bool done();
	void wait();

	enum ePriority
	{
		kIdle = 0,
		kLow = 1,
		kNormal = 2,
		kHigh = 3,
		kCritical = 4
	};
	void setPriority(ePriority priority);
	void suspend();
	void resume();

	const std::string &getName() const;
	void setName(const std::string &i_name);

	static Thread * getCurrentThread();
	static Thread * getMainThread();
protected:
	virtual ~Thread();
	virtual void run()=0;

	HANDLE handle;
	unsigned int id;
	InterlockedInteger refcount;
public: // should be private:, see above
	Thread(const Thread &o);
	Thread &operator =(const Thread &o);

	void start();
	void attach(); // pick up the currently running thread

	void ref();
	void deref();

	std::string *name;

	static int implindex;
	static Thread * mainThread;
	static unsigned int __stdcall threadFunc(void * data);
};

inline void Thread::ref()
{
	++refcount;
}

inline void Thread::deref()
{
	if (--refcount == 0) 
		delete this;
}

inline Thread * Thread::getCurrentThread()
{
	return static_cast<Thread *>(TlsGetValue(implindex));
}

inline Thread * Thread::getMainThread()
{
	return mainThread;
}

#endif