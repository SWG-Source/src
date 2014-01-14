// ======================================================================
//
// Gate.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Gate_h
#define INCLUDED_Gate_h

#include "sharedSynchronization/ConditionVariable.h"

class Gate
{
public:
	Gate(bool open);
	~Gate();

	void wait();

	void close();
	void open();
private:
	Gate(const Gate &o);
	Gate &operator =(const Gate &o);

	Mutex lock;
	bool opened;
	ConditionVariable cond;
};


#endif
