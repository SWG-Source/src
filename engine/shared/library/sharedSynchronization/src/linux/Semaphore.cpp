// ======================================================================
//
// Semaphore.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedSynchronization/Semaphore.h"

Semaphore::Semaphore(int count, int initial)
{
	sem_init(&sem, false, initial);
}

Semaphore::~Semaphore()
{
	sem_destroy(&sem);
}

void Semaphore::wait()
{
	sem_wait(&sem);
}

void Semaphore::wait(unsigned int maxDurationMs)
{
	timespec ts;
	ts.tv_sec = maxDurationMs/1000000;
	ts.tv_nsec = maxDurationMs%1000000;
	sem_timedwait(&sem, &ts);
}

void Semaphore::signal(int count)
{
	sem_post(&sem);
}

