// ======================================================================
//
// Semaphore.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedSynchronization/FirstSharedSynchronization.h"
#include "sharedSynchronization/Semaphore.h"

Semaphore::Semaphore(int count, int initial)
{
	handle = CreateSemaphore(0, initial, count, 0);
}

Semaphore::~Semaphore()
{
	CloseHandle(handle);
}

void Semaphore::wait()
{
	WaitForSingleObject(handle, INFINITE);
}

void Semaphore::wait(unsigned int maxDurationMs)
{
	WaitForSingleObject(handle, maxDurationMs);
}

void Semaphore::signal(int count)
{
	ReleaseSemaphore(handle, count, 0);
}

