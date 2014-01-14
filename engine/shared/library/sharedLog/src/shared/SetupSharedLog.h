// ======================================================================
//
// SetupSharedLog.h
//
// Copyright 2002-2003 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SetupSharedLog_H
#define INCLUDED_SetupSharedLog_H

// ======================================================================

class SetupSharedLog
{
public:

	static void install(std::string const &procId, bool flushOnWrite = true);
	static void remove();

private:

	SetupSharedLog(SetupSharedLog const &);
	SetupSharedLog &operator=(SetupSharedLog const &);
};

// ======================================================================

#endif

