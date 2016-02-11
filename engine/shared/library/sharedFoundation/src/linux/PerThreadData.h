// ======================================================================
//
// PerThreadData.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef PER_THREAD_DATA_H
#define PER_THREAD_DATA_H

// ======================================================================

#include "sharedFoundation/ExitChain.h"

class Gate;

// ======================================================================
/**
 * Provide thread local storage functionality.
 *
 * This class' purpose is to allow each thread to maintain some storage that is local and private to each thread.
 * The system must be installed before use.  Each thread that may use per-thread-data will also need to call the
 * threadInstall() routine after creation and threadRemove() just before termination of the thread.
 */
class PerThreadData
{
private:

	struct Data
	{
		bool              exitChainRunning;
		bool              exitChainFataling;
		ExitChain::Entry *exitChainFirstEntry;

		int               debugPrintFlags;

		Gate             *readGate;
	};

	static pthread_key_t slot;
	static bool slotCreated;

private:

	static Data *getData(bool allowReturnNull=false);

private:

	PerThreadData(void);
	PerThreadData(const PerThreadData &);
	PerThreadData &operator =(const PerThreadData &);

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

	static Gate *getFileStreamerReadGate(void);
};

// ======================================================================
/**
 * Determine if the per-thread-data is available for this thread
 *
 * Return value:
 *
 *   True if the per-thread-data is installed correctly, false otherwise
 *
 * Remarks:
 *
 *   This routine is not intended for general use; it should only be used by the ExitChain class.
 *
 *   -TRF- looks like Win98 does not zero out a new TLS slot for
 *         threads existing at the time of slot creation.  Thus, if you build a
 *         plugin that only initializes the engine the first time it is
 *         used, and other threads already exist in the app, those threads
 *         will contain bogus non-nullptr data in the TLS slot.  If the plugin really
 *         wants to do lazy initialization of the engine, it will need
 *         to handle calling PerThreadData::threadInstall() for all existing threads
 *         (except the thread that initialized the engine, which already
 *         has its PerThreadData::threadInstall() called).
 */

inline bool PerThreadData::isThreadInstalled(void)
{
	return (getData(true) != nullptr);
}

// ----------------------------------------------------------------------
/**
 * Get the exit chain running flag value.
 *
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 *
 * @return True if the exit chain is running, false otherwise.
 * @see ExitChain::isRunning()
 */

inline bool PerThreadData::getExitChainRunning(void)
{
	return getData()->exitChainRunning;
}

// ----------------------------------------------------------------------
/**
 * Set the exit chain running flag value.
 *
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 *
 * @param newValue  New value for the exit chain running flag
 */

inline void PerThreadData::setExitChainRunning(bool newValue)
{
	getData()->exitChainRunning = newValue;
}

// ----------------------------------------------------------------------
/**
 * Get the exit chain fataling flag value.
 *
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 *
 * @return True if the exit chain is fataling, false otherwise.
 * @see ExitChain::isFataling()
 */

inline bool PerThreadData::getExitChainFataling(void)
{
	return getData()->exitChainFataling;
}

// ----------------------------------------------------------------------
/**
 * Set the exit chain fataling flag value.
 *
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 *
 * @param newValue  New value for the exit chain fataling flag
 */

inline void PerThreadData::setExitChainFataling(bool newValue)
{
	getData()->exitChainFataling = newValue;
}

// ----------------------------------------------------------------------
/**
 * Get the first entry for the exit chain.
 *
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 * This routine may return nullptr.
 *
 * @return Pointer to the first entry on the exit chain
 * @see ExitChain::isFataling()
 */

inline ExitChain::Entry *PerThreadData::getExitChainFirstEntry(void)
{
	return getData()->exitChainFirstEntry;
}

// ----------------------------------------------------------------------
/**
 * Set the exit chain fataling flag value.
 *
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 * The parameter to this routine may be nullptr.
 *
 * @param newValue  New value for the exit chain first entry
 */

inline void PerThreadData::setExitChainFirstEntry(ExitChain::Entry *newValue)
{
	getData()->exitChainFirstEntry = newValue;
}

// ----------------------------------------------------------------------
/**
 * Get the debug print flags.
 *
 * This routine is not intended for general use; it should only be used by the DebugPrint functions.
 *
 * @return Current value of the debug print flags
 */

inline int PerThreadData::getDebugPrintFlags(void)
{
	return getData()->debugPrintFlags;
}

// ----------------------------------------------------------------------
/**
 * Set the debug print flags value.
 *
 * This routine is not intended for general use; it should only be used by the DebugPrint functions.
 */

inline void PerThreadData::setDebugPrintFlags(int newValue)
{
	getData()->debugPrintFlags = newValue;
}

// ----------------------------------------------------------------------

inline Gate *PerThreadData::getFileStreamerReadGate(void)
{
	return getData()->readGate;
}

// ======================================================================

#endif
