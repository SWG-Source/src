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
	HANDLE handle;
};

#endif
