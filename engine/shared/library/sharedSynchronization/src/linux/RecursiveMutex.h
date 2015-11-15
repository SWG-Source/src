// ======================================================================
//
// RecursiveMutex.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RecursiveMutex_h
#define INCLUDED_RecursiveMutex_h

#include "sharedSynchronization/InterlockedInteger.h"

class RecursiveMutex
{
public:
	RecursiveMutex();
	~RecursiveMutex();

	void enter();
	void leave();
private:
	RecursiveMutex(const RecursiveMutex &o);
	RecursiveMutex &operator =(const RecursiveMutex &o);

	pthread_mutex_t mutex;
};

#endif

