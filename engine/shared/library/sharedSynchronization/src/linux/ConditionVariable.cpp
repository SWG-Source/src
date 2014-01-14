// ======================================================================
//
// ConditionVariable.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedSynchronization/ConditionVariable.h"

ConditionVariable::ConditionVariable(Mutex &m)
  : _mutex(m)
{
	pthread_cond_init(&cond, 0);
}

ConditionVariable::~ConditionVariable()
{
	pthread_cond_destroy(&cond);
}

void ConditionVariable::wait()
{
	pthread_cond_wait(&cond, &_mutex.getInternalMutex());
}

void ConditionVariable::signal()
{
	pthread_cond_signal(&cond);
}

void ConditionVariable::broadcast()
{
	pthread_cond_broadcast(&cond);
}

