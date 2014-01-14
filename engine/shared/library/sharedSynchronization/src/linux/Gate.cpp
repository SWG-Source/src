// ======================================================================
//
// Gate.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedSynchronization/Gate.h"

Gate::Gate(bool open)
: cond(lock)
{
	opened = open;
}

Gate::~Gate()
{
}

void Gate::wait()
{
	lock.enter();
	while (!opened)
		cond.wait();
	lock.leave();
}

void Gate::close()
{
	lock.enter();
	opened = false;
	lock.leave();
}

void Gate::open()
{
	lock.enter();
	opened = true;
	cond.broadcast();
	lock.leave();
}

