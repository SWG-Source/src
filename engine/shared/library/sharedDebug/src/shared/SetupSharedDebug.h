// ======================================================================
//
// SetupSharedDebug.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedDebug_H
#define INCLUDED_SetupSharedDebug_H

// ======================================================================

class SetupSharedDebug
{
public:

	static void install(int maxProfilerEntries);

private:
	SetupSharedDebug();
	SetupSharedDebug(const SetupSharedDebug &);
	SetupSharedDebug &operator =(const SetupSharedDebug &);
};

// ======================================================================

#endif
