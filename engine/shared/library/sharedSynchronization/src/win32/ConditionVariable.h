// ======================================================================
//
// ConditionVariable.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConditionVariable_h
#define INCLUDED_ConditionVariable_h

class Mutex;

#include "sharedSynchronization/Semaphore.h"

/* Condition variable semantics follow pthreads semantics.  You
must have the mutex locked before calling signal or broadcast.
When wait returns, the mutex is locked and you must unlock it.
*/

class ConditionVariable
{
public:
	ConditionVariable(Mutex &m);
	~ConditionVariable();
	void wait();
	// You must own the mutex before calling signal.
	void signal();
	void broadcast();
	Mutex &mutex() { return _mutex; }
private:
	ConditionVariable(const ConditionVariable &o);
	ConditionVariable &operator =(const ConditionVariable &o);

	Semaphore sem;
	Mutex &_mutex;
	int count;
};


#endif