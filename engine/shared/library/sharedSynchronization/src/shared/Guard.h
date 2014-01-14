// ======================================================================
//
// Guard.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _Guard_H_
#define _Guard_H_

// ======================================================================

#include "sharedSynchronization/RecursiveMutex.h"

// ======================================================================

class Guard
{
public:
	Guard(RecursiveMutex &mutex) :
		m_mutex(mutex)
	{
		mutex.enter();
	}
	~Guard()
	{
		m_mutex.leave();
	}

private:
	Guard(Guard const &);
	Guard &operator=(Guard const &);

private:
	RecursiveMutex &m_mutex;
};

// ======================================================================

#endif // _Guard_H_

