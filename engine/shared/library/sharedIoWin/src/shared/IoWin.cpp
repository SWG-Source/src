// ======================================================================
//
// IoWin.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedIoWin/FirstSharedIoWin.h"
#include "sharedIoWin/IoWin.h"

#include "sharedIoWin/IoWinManager.h"

// ======================================================================
// Cosntruct an IoWin
//
// Remarks:
//
//   The debugName argument is used only for debugging

IoWin::IoWin(
	const char *debugName  // Name used for debugging purposes
	)
: ioDebugName(DuplicateString(debugName)),
	ioNext(nullptr)
{

}

// ----------------------------------------------------------------------
/**
 * Destroy an IoWin.
 * 
 * This routine is intended to be overloaded in derived classes.
 */

IoWin::~IoWin(void)
{
	delete [] ioDebugName;
	ioDebugName = nullptr;

	ioNext = nullptr;
}

// ----------------------------------------------------------------------
/**
 * Process an event.
 * 
 * This routine is intended to be overloaded in derived classes.
 * 
 * The event is intentionally not const so that a IoWin may modify the event
 * for other IoWins that are further down the stack.
 * 
 * If this routine returns IOR_Pass or IOR_PassKillMe, the event will be passed
 * down to the next window on the stack.  If this routine returns IOR_Block or
 * IOR_BlockKillMe, the event will not be passed down to the next window on the
 * stack, and will effectively be squelched.
 * 
 * If this routine returns IOR_PassKillMe or IOR_BlockKillMe, the window will
 * first be closed, and then will be deleted.
 * 
 * This base-class version simply returns IOR_Pass.
 * 
 * @param event  The event to process
 * @return How the IoWinManager should handle this window and event
 */

IoResult IoWin::processEvent(IoEvent *event)
{
	UNREF(event);

	return IOR_Pass;
}

// ----------------------------------------------------------------------
/**
 * Draw the window.
 * 
 * This routine is intended to be overloaded in derived classes.
 * 
 * This base-class version simply calls the next IoWin's draw routine.  Derived classes
 * may or may not want to call this routine in the base class, depending on their needs.
 */

void IoWin::draw(void) const
{
	if (ioNext)
		ioNext->draw();
}

// ----------------------------------------------------------------------
/**
 * Open this window.
 * 
 * This routine simply calls IoWinManager::open() with itself as the argument.
 */

void IoWin::open(void)
{
	IoWinManager::open(this);
}

// ----------------------------------------------------------------------
/**
 * Close this window.
 * 
 * This routine simply calls IoWinManager::close() with itself as the argument.
 */

void IoWin::close(void)
{
	IoWinManager::close(this);
}

// ======================================================================


