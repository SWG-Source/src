// ======================================================================
//
// CountingSemaphore.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CountingSemaphore_h
#define INCLUDED_CountingSemaphore_h

#include "sharedSynchronization/Semaphore.h"
// this needs a little more thought

class CountingSemaphore
{
public:
	CountingSemaphore();
	~CountingSemaphore();

	void wait() { ++waiters; sem.wait(); }
	void signal(int i_count=1)
	{
		if (i_count > waiters) i_count = waiters;
		sem.signal(i_count);
	}

	int count() { return waiters; }
private:
	CountingSemaphore(const CountingSemaphore &o);
	CountingSemaphore &operator =(const CountingSemaphore &o);
	Semaphore sem;
	int waiters;
};


#endif