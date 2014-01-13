// ======================================================================
//
// PerThreadData.cpp
//
// Portions copyright 1998 Bootprint Entertainment
// Portions Copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/PerThreadData.h"

#include "sharedSynchronization/RecursiveMutex.h"
#include "sharedSynchronization/Gate.h"

#include "sharedFoundation/Os.h"

#include <winbase.h>
#include <vector>

// ======================================================================

// ======================================================================
namespace PerThreadDataNamespace
{
	struct Data
	{
		bool              exitChainRunning;
		bool              exitChainFataling;
		ExitChain::Entry *exitChainFirstEntry;

		int               debugPrintFlags;

		Gate             *fileStreamerReadGate;

		HANDLE            watchHandle;
	};

	typedef std::vector<Data *> DataAllocations;

	// ------------------------------------------------------

	static char buffer1[sizeof(RecursiveMutex)];
	static char buffer2[sizeof(DataAllocations)];

	static const DWORD BadSlot = 0xffffffff;

	static DWORD                             slot = BadSlot;
	static RecursiveMutex                   *criticalSection;
	static DataAllocations                  *dataAllocations;

	// ------------------------------------------------------

	static void _threadRemove(Data *data);
	static void _removeFromList(Data *data);
	static void _watchThreads();

	/*----------------------------------------------------------------------
	 * Get access to the per-thread-data.
	 * 
	 * This routine will verify the per-thread-data subsystem has been installed and the
	 * threadInstall() function has been called for the current thread.
	 * 
	 * @return A pointer to the per-thread-data.
	 */
	inline static Data *_getData(bool allowReturnNull=false)
	{
		UNREF(allowReturnNull);

		if (slot == BadSlot)
		{
			DEBUG_FATAL(true && !allowReturnNull, ("not installed"));
			return NULL;
		}

		Data * const data = reinterpret_cast<Data *>(TlsGetValue(slot));
		DEBUG_FATAL(!data && !allowReturnNull, ("not installed for this thread"));
		return data;
	}

	// ------------------------------------------------------

	inline static void _closeData(Data *data)
	{
		if (data->fileStreamerReadGate)
		{
			delete data->fileStreamerReadGate;
			data->fileStreamerReadGate=0;
		}
		if (data->watchHandle)
		{
			CloseHandle(data->watchHandle);
			data->watchHandle=0;
		}
	}

	// ------------------------------------------------------

	inline static void _freeData(Data *data)
	{
		delete data;
	}

	// ------------------------------------------------------
}
using namespace PerThreadDataNamespace;

// ======================================================================

// ======================================================================
// Install the per-thread-data subsystem
//
// Remarks:
//
//   This routine will install the per-thread-data subsystem that is required for several
//   other subsystems in the engine.  It should be called from the primary thread before
//   any other threads have been created.  It will also call threadInstall() for the
//   primary thread.
//
// See Also:
//
//   PerThreadData::remove()

void PerThreadData::install()
{
	slot = TlsAlloc();
	FATAL(slot == BadSlot, ("TlsAlloc failed"));

	criticalSection = new(buffer1) RecursiveMutex;
	dataAllocations = new(buffer2) DataAllocations;
	dataAllocations->reserve(8);
	threadInstall();
}

// ----------------------------------------------------------------------
/**
 * Remove the per-thread-subsystem.
 * 
 * This routine should be called by the primary thread after all other threads have
 * terminated, and no other uses of per-thread-data will occur.
 * 
 * @see PerThreadData::install()
 */
void PerThreadData::remove()
{
	criticalSection->enter();
	{
		// remove this thread.
		threadRemove();

		// clean up after any threads which didn't clean up after themselves (like the miles sound system)
		const DataAllocations::iterator iEnd = dataAllocations->end();
		for (DataAllocations::iterator i = dataAllocations->begin(); i != iEnd; ++i)
		{
			_closeData(*i);
			_freeData(*i);
		}

		dataAllocations->clear();
	}
	criticalSection->leave();

	criticalSection->~RecursiveMutex();
	dataAllocations->~vector();

	memset(buffer1, 0, sizeof(buffer1));
	memset(buffer2, 0, sizeof(buffer2));

	const BOOL result = TlsFree(slot);
	FATAL(!result, ("TlsFree failed"));
}

// ----------------------------------------------------------------------
/**
 * Create the per-thread-data for a new thread.
 * 
 * This routine should be called in a thread before the first usage of per-thread-data.
 * 
 * If the client is calling this function on a thread that existed before the engine was
 * installed, the client should set isNewThread to false.  Setting isNewThread to false
 * prevents the function from validating that the thread's TLS is NULL.  For threads existing
 * before the engine is installed, the TLS data for the thread is undefined.
 * 
 * @param isNewThread  [IN] true if the thread was created after the engine was installed, false otherwise
 * @see PerThreadData::threadRemove()
 */

void PerThreadData::threadInstall(bool isNewThread)
{
	UNREF(isNewThread);

	DEBUG_FATAL(slot == BadSlot, ("not installed"));

	// only check for already-set data if this is supposed to be a new thread
	DEBUG_FATAL(isNewThread && TlsGetValue(slot), ("already installed for this thread"));

	// create the data
	Data * const data = new Data;

	// initialize the data
	memset(data, 0, sizeof(*data));

	//create the event for file streaming reads
	data->fileStreamerReadGate = new Gate(false);

	// set the data into the thread slot
	const BOOL result = TlsSetValue(slot, data);
	UNREF(result);
	DEBUG_FATAL(!result, ("TlsSetValue failed"));

	criticalSection->enter();
	{
		_watchThreads();

		dataAllocations->push_back(data);

		// This is most likely a thread created by a 3rd party library such as Miles or Bink.
		// Add a watch handle to it so we can clean it up.
		if (!isNewThread)
		{
			const HANDLE processHandle = GetCurrentProcess();
			DuplicateHandle(processHandle, GetCurrentThread(), processHandle, &data->watchHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
		}
	}
	criticalSection->leave();
}

// ----------------------------------------------------------------------

void PerThreadDataNamespace::_removeFromList(Data *data)
{
	const DataAllocations::iterator iEnd = dataAllocations->end();
	const DataAllocations::iterator i    = std::find(dataAllocations->begin(), iEnd, data);
	if (i!=iEnd)
	{
		dataAllocations->erase(i);
	}
}

// ----------------------------------------------------------------------
// Synchronization is left up to the user 
// Unless this is called during shutdown, the critical section
// needs to be entered.
void PerThreadDataNamespace::_watchThreads()
{
	DataAllocations::iterator tiDest = dataAllocations->begin();
	DataAllocations::iterator tiSrc = tiDest;
	DataAllocations::iterator tiEnd = dataAllocations->end();

	while (tiSrc!=tiEnd)
	{
		Data *const data = *tiSrc;
		const HANDLE h = data->watchHandle;

		if (h!=0 && WaitForSingleObject(h, 0)==WAIT_OBJECT_0)
		{
			_closeData(data);
			_freeData(data);
		}
		else
		{
			*tiDest++=data;
		}
		++tiSrc;
	}

	if (tiSrc!=tiDest)
	{
		const int newSize = tiDest - dataAllocations->begin();
		dataAllocations->resize(newSize);
	}
}

// ----------------------------------------------------------------------

void PerThreadDataNamespace::_threadRemove(Data *data)
{
	//close the event used for file streaming reads
	_closeData(data);

	_removeFromList(data);

	// free the memory
	_freeData(data);
}

// ----------------------------------------------------------------------
/**
 * Destroy the per-thread-data for a terminating thread.
 * 
 * This routine should be called in a thread after the last usage of per-thread-data.
 * 
 * @see PerThreadData::threadInstall()
 */

void PerThreadData::threadRemove()
{
	DEBUG_FATAL(slot == BadSlot, ("not installed"));
	DEBUG_FATAL(!TlsGetValue(slot), ("thread not installed"));

	// find our thread record and free the memory
	criticalSection->enter();

		_threadRemove(_getData());

		// wipe the data in the thread slot
		const BOOL result = TlsSetValue(slot, NULL);
		UNREF(result);
		DEBUG_FATAL(!result, ("TlsSetValue failed"));

	criticalSection->leave();
}

// ======================================================================

// ======================================================================
// Determine if the per-thread-data is available for this thread
//
// Return value:
//
//   True if the per-thread-data is installed correctly, false otherwise
//
// Remarks:
//
//   This routine is not intended for general use; it should only be used by the ExitChain class.
//
//   -TRF- looks like Win98 does not zero out a new TLS slot for
//         threads existing at the time of slot creation.  Thus, if you build a
//         plugin that only initializes the engine the first time it is
//         used, and other threads already exist in the app, those threads
//         will contain bogus non-null data in the TLS slot.  If the plugin really
//         wants to do lazy initialization of the engine, it will need
//         to handle calling PerThreadData::threadInstall() for all existing threads
//         (except the thread that initialized the engine, which already
//         has its PerThreadData::threadInstall() called).

bool PerThreadData::isThreadInstalled()
{
	return (_getData(true) != NULL);
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

bool PerThreadData::getExitChainRunning()
{
	return _getData()->exitChainRunning;
}

// ----------------------------------------------------------------------
/**
 * Set the exit chain running flag value.
 * 
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 * 
 * @param newValue  New value for the exit chain running flag
 */

void PerThreadData::setExitChainRunning(bool newValue)
{
	_getData()->exitChainRunning = newValue;
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

bool PerThreadData::getExitChainFataling()
{
	return _getData()->exitChainFataling;
}

// ----------------------------------------------------------------------
/**
 * Set the exit chain fataling flag value.
 * 
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 * 
 * @param newValue  New value for the exit chain fataling flag
 */

void PerThreadData::setExitChainFataling(bool newValue)
{
	_getData()->exitChainFataling = newValue;
}

// ----------------------------------------------------------------------
/**
 * Get the first entry for the exit chain.
 * 
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 * This routine may return NULL.
 * 
 * @return Pointer to the first entry on the exit chain
 * @see ExitChain::isFataling()
 */

ExitChain::Entry *PerThreadData::getExitChainFirstEntry()
{
	return _getData()->exitChainFirstEntry;
}

// ----------------------------------------------------------------------
/**
 * Set the exit chain fataling flag value.
 * 
 * This routine is not intended for general use; it should only be used by the ExitChain class.
 * The parameter to this routine may be NULL.
 * 
 * @param newValue  New value for the exit chain first entry
 */

void PerThreadData::setExitChainFirstEntry(ExitChain::Entry *newValue)
{
	_getData()->exitChainFirstEntry = newValue;
}

// ----------------------------------------------------------------------
/**
 * Get the debug print flags.
 * 
 * This routine is not intended for general use; it should only be used by the DebugPrint functions.
 * 
 * @return Current value of the debug print flags
 */

int PerThreadData::getDebugPrintFlags()
{
	return _getData()->debugPrintFlags;
}

// ----------------------------------------------------------------------
/**
 * Set the debug print flags value.
 * 
 * This routine is not intended for general use; it should only be used by the DebugPrint functions.
 */

void PerThreadData::setDebugPrintFlags(int newValue)
{
	_getData()->debugPrintFlags = newValue;
}

// ----------------------------------------------------------------------

Gate *PerThreadData::getFileStreamerReadGate()
{
	return _getData()->fileStreamerReadGate;
}

// ======================================================================
