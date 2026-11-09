// ======================================================================
//
// Semaphore.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Semaphore_h
#define INCLUDED_Semaphore_h

// A case-insensitive shared source directory can resolve <semaphore.h> to this
// file. Continue past our include directory to load the POSIX declarations.
#include_next <semaphore.h>

class Semaphore
{
public:
	Semaphore(int count=0x7FFFFFFF, int initial=0);
	~Semaphore();

	void wait();
	void wait(unsigned int maxDurationMs);
	void signal(int count=1);
private:
	Semaphore(const Semaphore &o);
	Semaphore &operator =(const Semaphore &o);

	sem_t sem;
};

#endif

