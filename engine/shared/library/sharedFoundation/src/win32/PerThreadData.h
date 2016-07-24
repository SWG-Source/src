// ======================================================================
//
// PerThreadData.h
//
// Portions copyright 1998 Bootprint Entertainment
// Portions Copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_PerThreadData_H
#define INCLUDED_PerThreadData_H

// ======================================================================

class Gate;

#include "sharedFoundation/ExitChain.h"

// ======================================================================

// Provide thread local storage functionality.
//
// This class' purpose is to allow each thread to maintain some storage that is local and private to each thread.
// The system must be installed before use.  Each thread that may use per-thread-data will also need to call the
// threadInstall() routine after creation and threadRemove() just before termination of the thread.

class PerThreadData
{
public:

	static void install(void);
	static void remove(void);

	static void threadInstall(bool isNewThread = true);
	static void threadRemove(void);

	static bool isThreadInstalled(void);

	static bool getExitChainRunning(void);
	static void setExitChainRunning(bool newValue);

	static bool getExitChainFataling(void);
	static void setExitChainFataling(bool newValue);

	static ExitChain::Entry *getExitChainFirstEntry(void);
	static void              setExitChainFirstEntry(ExitChain::Entry *newValue);

	static int  getDebugPrintFlags(void);
	static void setDebugPrintFlags(int newValue);

	static Gate   *getFileStreamerReadGate(void);
};

// ======================================================================

#endif
