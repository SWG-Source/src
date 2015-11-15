// ======================================================================
//
// ConditionVariable.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSynchronization/FirstSharedSynchronization.h"
#include "sharedSynchronization/ConditionVariable.h"

#include "sharedSynchronization/Mutex.h"

ConditionVariable::ConditionVariable(Mutex &m)
: sem(1), _mutex(m), count(0)
{
}

ConditionVariable::~ConditionVariable()
{
}

void ConditionVariable::wait()
{
	++count;
	_mutex.leave();
	sem.wait();
	_mutex.enter();
}

void ConditionVariable::signal()
{
	if (count)
	{
		--count;
		sem.signal();
	}
}

void ConditionVariable::broadcast()
{
	if (count)
	{
		int oldcount = count;
		count = 0;
		sem.signal(oldcount);
	}
}

