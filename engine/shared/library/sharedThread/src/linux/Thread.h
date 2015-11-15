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
#include <string>

class Thread
{
  template <class T> friend class TypedThreadHandle;
public:
	static void install();
	static void remove();

	Thread(const std::string &i_name=std::string());

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

	const std::string &getName() const;
	void setName(const std::string &i_name);
			
	static Thread * getCurrentThread();
	static Thread * getMainThread();
protected:
	virtual ~Thread();
	virtual void run()=0;

	pthread_t thread;

	InterlockedInteger refcount;
private:
	Thread(const Thread &o);
	Thread &operator =(const Thread &o);

	void start();
	void attach(); // pick up the currently running thread

	void ref();
	void deref();

	bool doneFlag;

	std::string name;

	static pthread_key_t implindex;
	static Thread * mainThread;
	static void * threadFunc(void * data);
};

inline void Thread::ref()
{
	++refcount;
}

inline void Thread::deref()
{
	if (--refcount == 0) delete this;
}

inline Thread * Thread::getCurrentThread()
{
	return static_cast<Thread *>(pthread_getspecific(implindex));
}

inline Thread * Thread::getMainThread()
{
	return mainThread;
}

inline const std::string &Thread::getName() const
{
	return name;
}

inline void Thread::setName(const std::string &i_name)
{
	name = i_name;
}

#endif
