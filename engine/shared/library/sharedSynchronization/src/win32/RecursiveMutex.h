// ======================================================================
//
// RecursiveMutex.h
// Copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RecursiveMutex_H
#define INCLUDED_RecursiveMutex_H

// ======================================================================

class RecursiveMutex
{
public:

	static void install();
	static void remove();

public:

	RecursiveMutex();
	~RecursiveMutex();

	void enter();
	void leave();

private:

	RecursiveMutex(const RecursiveMutex &);
	RecursiveMutex &operator =(const RecursiveMutex &);

private:

	CRITICAL_SECTION m_criticalSection;
};

// ======================================================================

#endif
