// ======================================================================
//
// DebugHelp.h
// copyright 2000 Verant Interactive
//
// ======================================================================

#ifndef DEBUG_HELP_H
#define DEBUG_HELP_H

#include "sharedDebug/FirstSharedDebug.h"

// ======================================================================

class DebugHelp
{
public:

	static void install();
	static void remove();

	//-- Entries are instruction addresses, so the buffer must be pointer-width
	//   capable. uint64 matches lookupAddress() below and is wide enough on
	//   both ILP32 and LP64.
	static void getCallStack(uint64 *callStack, int sizeOfCallStack);
	static bool lookupAddress(uint64 address, char *libName, char *fileName, int fileNameLength, int &line);
};

// ======================================================================

#endif

