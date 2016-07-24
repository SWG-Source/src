// ======================================================================
//
// Mutex.h
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Mutex_H
#define INCLUDED_Mutex_H

// ======================================================================

class Mutex
{
public:

	static void install();
	static void remove();

public:

	Mutex();
	~Mutex();

	void enter();
	void leave();

private:

	Mutex(const Mutex &);
	Mutex &operator =(const Mutex &);

private:

	CRITICAL_SECTION m_criticalSection;
};

// ======================================================================

#endif
