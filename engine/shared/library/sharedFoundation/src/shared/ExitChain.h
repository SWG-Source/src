// ======================================================================
//
// ExitChain.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// Maintains a list of functions to call at game exit time for cleanup.
//
// Functions are first in order of priority, with the higher interger priority functions being called
// first.  For functions with the same priority, they are called in the opposite order that they were
// added to the ExitChain, in a last-in-first-out (LIFO) manner.
//
// When the ExitChain is running because fatal() was called, only the entries that have their critical flag
// set true will be called.
//
// ======================================================================

#ifndef EXIT_CHAIN_H
#define EXIT_CHAIN_H

// ======================================================================

#include "sharedDebug/CallStack.h"

// ======================================================================

class ExitChain
{
public:

	typedef void (*Function)(void);

	struct Entry
	{
		Entry      *next;
		Function    function;
		const char *name;
		int         priority;
		bool        critical;
#ifdef _DEBUG
		CallStack m_callStack;
#endif
	};

protected:

	// disabled
	ExitChain(void);
	ExitChain(const ExitChain &);
	ExitChain &operator =(const ExitChain &);

private:

	static void run(void);

private:

#ifdef _DEBUG
	static bool ms_debugReportFlag;
	static bool ms_debugLogFlag;
#endif

public:

	static void           install();
	static void           quit();
	static void           fatal();

	static void           add(Function function, const char *debugName, int priority=0, bool critical=false);
	static void           remove(Function function);


	static bool           isRunning(void);
	static DLLEXPORT bool isFataling(void);

	static void           debugReport();
	static void           debugReport(bool log);
};

// ======================================================================

#endif

