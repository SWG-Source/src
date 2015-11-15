// ======================================================================
//
// DebugHelp.h
// copyright 2000 Verant Interactive
//
// ======================================================================

#ifndef DEBUG_HELP_H
#define DEBUG_HELP_H

// ======================================================================

typedef unsigned long uint32;

// ======================================================================

class DebugHelp
{
public:

	static void install();
	static void remove();

	static bool loadSymbolsForDll(const char *name);

	static void getCallStack(uint32 *callStack, int sizeOfCallStack);
	static void reportCallStack(int const maxStackDepth = 4);
	static bool lookupAddress(uint32 address, char *libName, char *fileName, int fileNameLength, int &line);

	static bool writeMiniDump(char const *miniDumpFileName=0, PEXCEPTION_POINTERS exceptionPointers=0);
};

// ======================================================================

#endif
