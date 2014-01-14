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

#include "sharedSynchronization/Mutex.h"

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

	Mutex &_mutex;
	pthread_cond_t cond;
};


#endif
