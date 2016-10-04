// ======================================================================
//
// ExitChain.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ExitChain.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/PerThreadData.h"

// ======================================================================

#ifdef _DEBUG
bool ExitChain::ms_debugReportFlag;
bool ExitChain::ms_debugLogFlag;
#endif

// ======================================================================

void ExitChain::install()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugReportFlag, "SharedFoundation", "reportExitChain", &ExitChain::debugReport);
	DebugFlags::registerFlag(ms_debugLogFlag, "SharedFoundation", "logExitChain");
#endif
}

// ----------------------------------------------------------------------

void ExitChain::quit()
{
	run();
}

// ======================================================================
// Check if the ExitChain is running or not
//
// Return value:
//
//   True if ExitChain::quit() or ExitChain::fatal() has been called
//
// Remarks:
//
//   If the result is true, the application is terminating.
//
// See Also:
//
//   ExitChain::quit(), ExitChain:fatal(), ExitChain::isFataling()

bool ExitChain::isRunning(void)
{
	return PerThreadData::getExitChainRunning();
}

// ----------------------------------------------------------------------
/**
 * Check if the ExitChain is fataling or not.
 * 
 * If the ExitChain is fataling, minimal cleanup work should be done to restore the users system to a usable
 * state.  If this routine returns true, ExitChain::isRunning() will also return true.
 * 
 * @return True if the ExitChain::fatal() has been called
 * @see ExitChain::isRunning()
 */

bool ExitChain::isFataling(void)
{
	return PerThreadData::getExitChainFataling();
}

// ----------------------------------------------------------------------
/**
 * Add a function to the ExitChain to be called when the game exits.
 * 
 * The order of the function calls depends on priority and the order in which they are added to the
 * ExitChain.  See the main ExitChain documentation for more details.
 * 
 * @param function  The function to call
 * @param debugName  Text description of the function (for debugging purposes only)
 * @param priority  Priority of the function for proper shutdown ordering
 * @param critical  Flag to indicate the function should be called even when fataling
 * @see ExitChain
 */

void ExitChain::add(Function function, const char *debugName, int priority, bool critical)
{
#ifdef _DEBUG
	//-- Verify that the entry doesn't already exist in the ExitChain
	{
		for (Entry * entry = PerThreadData::getExitChainFirstEntry(); entry; entry = entry->next)
		{
			if (entry->function == function)
			{
				DEBUG_WARNING(true, ("ExitChain::add: %s is already in the list", debugName));
				entry->m_callStack.debugLog();
			}
		}
	}
#endif

	Entry *entry, *back, *front;

	// create the new ExitChain entry
	entry = new Entry;
	entry->function = function;
	entry->name     = debugName;
	entry->priority = priority;
	entry->critical = critical;
#ifdef _DEBUG
	entry->m_callStack.sample();
#endif
	
	// linked list traversal with a back pointer
	for (back = nullptr, front = PerThreadData::getExitChainFirstEntry(); front && front->priority > priority; back = front, front = front->next)
		;

	// hook it into the linked list
	entry->next = front;
	if (back)
		back->next = entry;
	else
		PerThreadData::setExitChainFirstEntry(entry);
}

// ----------------------------------------------------------------------
/**
 * Remove a function from the ExitChain.
 * 
 * The ExitChain will automatically remove a function from the ExitChain when it calls the function, so an
 * exit function should not attempt to remove itself from the ExitChain.
 * 
 * Calling this routine with a nullptr pointer will cause this routine to call Fatal in debug compilations.
 * 
 * Calling this routine with a function that is not on the ExitChain will cause this routine to call
 * Fatal in debug compilations.
 * 
 * @param function  The function to remove from the ExitChain list
 */

void ExitChain::remove(Function function)
{
	Entry *back, *front;

	if (function == nullptr)
	{
		DEBUG_FATAL(true, ("ExitChain::remove nullptr function"));
		return;  //lint !e527 // Warning -- Unreachable
	}

	// linked list traversal with a back pointer
	for (back = nullptr, front = PerThreadData::getExitChainFirstEntry(); front && front->function != function; back = front, front = front->next)
		;

	// make sure it was found
	if (!front)
	{
		DEBUG_FATAL(true, ("ExitChain::remove function not found %p", function));
		return; //lint !e527 // Warning -- Unreachable
	}

	// remove from the linked list
	if (back)
		back->next = front->next;
	else
		PerThreadData::setExitChainFirstEntry(front->next);

	// free the memory associated with this entry
	delete front;
}

// ----------------------------------------------------------------------
/**
 * Process the exit chain.
 * 
 * This routine is protected against re-entry so that the ExitChain routines can call Fatal and behave
 * properly.
 * 
 * @see Fatal()
 */

void ExitChain::run(void)
{
	Entry *entry;

	if (isRunning())
		return;

	PerThreadData::setExitChainRunning(true);

	while ((entry = PerThreadData::getExitChainFirstEntry()) != nullptr)
	{
		// remove the first entry off the ExitChain
		PerThreadData::setExitChainFirstEntry(entry->next);

		// call the function if it's critical or we're not fataling
		if (entry->critical || !isFataling())
		{
			DEBUG_REPORT_LOG_PRINT(ms_debugLogFlag, ("ExitChain::run >> %s\n", entry->name));
			DEBUG_OUTPUT_CHANNEL("Foundation\\Exitchain", ("ExitChain::run >> %s\n", entry->name));
			entry->function();
			DEBUG_REPORT_LOG_PRINT(ms_debugLogFlag, ("ExitChain::run << %s\n", entry->name));
			DEBUG_OUTPUT_CHANNEL("Foundation\\Exitchain", ("ExitChain::run << %s\n", entry->name));
		}

		// free the memory if we're not fataling
		if (!isFataling())
			delete entry;
	}
}

// ----------------------------------------------------------------------
/**
 * Clean up the game due to an abnormal condition.
 * 
 * This routine should only be called by Fatal().  It is not intended for any other use.
 * 
 * @see ExitChain::quit(), Fatal()
 */

void ExitChain::fatal(void)
{
	if (PerThreadData::isThreadInstalled())
	{
		PerThreadData::setExitChainFataling(true);
		run();
	}
}

// ----------------------------------------------------------------------
/**
 * Dump the contents of the ExitChain to the DebugMonitor.
 * 
 * This routine is for debugging only.
 * 
 * @param logMessage  Log to the debugger as well
 */

void ExitChain::debugReport(bool logMessage)
{
	UNREF(logMessage);

#ifdef _DEBUG

	DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME("Foundation\\Exitchain");
	for (Entry *entry = PerThreadData::getExitChainFirstEntry(); entry; entry = entry->next)
	{
		DEBUG_REPORT(true, Report::RF_print | (logMessage ? Report::RF_log : 0), ("%4d=pri %d=crit %p=func %s\n", entry->priority, static_cast<int>(entry->critical), entry->function, entry->name));
		DEBUG_OUTPUT_STATIC_VIEW("Foundation\\Exitchain", ("%4d=pri %d=crit %p=func %s\n", entry->priority, static_cast<int>(entry->critical), entry->function, entry->name));
	}
	DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME("Foundation\\Exitchain");
#endif
}

// ----------------------------------------------------------------------
/**
 * Dump the contents of the ExitChain to the DebugMonitor.
 * 
 * This routine is for debugging only.
 */

void ExitChain::debugReport()
{
	debugReport(false);
}

// ======================================================================

