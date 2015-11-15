// ======================================================================
//
// SetupSharedThread.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedThread_H
#define INCLUDED_SetupSharedThread_H

// ======================================================================

class SetupSharedThread
{
public:

	static void install();
	static void remove();

private:
	SetupSharedThread();
	SetupSharedThread(const SetupSharedThread &);
	SetupSharedThread &operator =(const SetupSharedThread &);
};

// ======================================================================

#endif
