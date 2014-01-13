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

typedef unsigned long uint32;

// ======================================================================

class DebugHelp
{
public:

	static void install();
	static void remove();

	static void getCallStack(uint32 *callStack, int sizeOfCallStack);
	static bool lookupAddress(uint32 address, char *libName, char *fileName, int fileNameLength, int &line);
};

// ======================================================================

#endif

