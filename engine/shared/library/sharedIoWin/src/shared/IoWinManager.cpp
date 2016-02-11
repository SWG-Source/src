// ======================================================================
//
// IoWinManager.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedIoWin/FirstSharedIoWin.h"
#include "sharedIoWin/IoWinManager.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedIoWin/IoWin.h"
#include "sharedFoundation/Timer.h"

// ======================================================================

namespace IoWinManagerNamespace
{
	bool s_discardIdenticalJoystickMotionEvents = false;

	int const s_numJoystickAxes = 10;
	int const s_numJoystickSliders = 10;
	int const s_numJoysticks = 3;

	float s_lastJoystickAxis[s_numJoysticks][s_numJoystickAxes];
	float s_lastJoystickSlider[s_numJoysticks][s_numJoystickSliders];

	float const s_joystickAxisIdenticalThreshold = 0.1f;
	float const s_joystickSliderIdenticalThreshold = 0.1f;
	
	// Inactivity timer.
	float const s_defaultInactivityTimeSeconds = 15.0f * 60.0f;
	IoWinManager::InactivityCallback s_inactivityCallback = nullptr;
	Timer s_inactivityTimer(s_defaultInactivityTimeSeconds);
	bool s_isInactive = true;

	bool activtyDetected(IoEventType eventType);
	void triggerInactive(bool inactive);
}

//----------------------------------------------------------------------

bool IoWinManagerNamespace::activtyDetected(IoEventType eventType)
{
	bool modifiesInactivityState = true;
	
	switch (eventType)
	{
	case IOET_KeyDown:
	case IOET_KeyUp:
	case IOET_JoystickMove:
	case IOET_JoystickButtonDown:
	case IOET_JoystickButtonUp:
	case IOET_JoystickPovHat:
	case IOET_MouseMove:
	case IOET_MouseButtonDown:
	case IOET_MouseButtonUp:
		modifiesInactivityState = true;
		break;
		
	default:
		modifiesInactivityState = false;
		break;
	}
	
	return modifiesInactivityState;
}
//----------------------------------------------------------------------

void IoWinManagerNamespace::triggerInactive(bool inactive)
{
	if (!boolEqual(s_isInactive, inactive))
	{
		if (s_inactivityCallback != nullptr) 
		{
			(*s_inactivityCallback)(inactive);
		}
		
		s_isInactive = inactive;
	}
}


using namespace IoWinManagerNamespace;

//----------------------------------------------------------------------

bool                              IoWinManager::installed;
bool                              IoWinManager::discardInput;

#ifdef _DEBUG
bool                              IoWinManager::debugReportFlag;
bool                              IoWinManager::debugReportEvents;
#endif

IoWin                            *IoWinManager::top;
IoWin                            *IoWinManager::captured;

IoEvent                          *IoWinManager::firstEvent;
IoEvent                          *IoWinManager::lastEvent;

MemoryBlockManager *IoWinManager::eventBlockManager;
IoWinManager::IMEFunctionPointer  IoWinManager::imeFunction = nullptr;

// ======================================================================
// Install the IoWinManager
//
// Remarks:
//
//   This routine installs the IoWinManager subsystem.  It will add IoWinManager::remove()
//   to the ExitChain.
//
// See Also:
//
//   IoWinManager::remove()

void IoWinManager::install()
{
	DEBUG_FATAL(installed, ("double install"));
	top = nullptr;
	captured = nullptr;
	ExitChain::add(IoWinManager::remove, "IoWinManager::remove");
	eventBlockManager = new MemoryBlockManager("IoWinManager eventBlockManager", true, sizeof(IoEvent), 0, 0, 0);
	installed = true;

	Os::setQueueCharacterHookFunction(queueCharacter);
	Os::setSetSystemMouseCursorPositionHookFunction(queueSetSystemMouseCursorPosition);
	Os::setQueueKeyDownHookFunction(queueKeyDown);

	Zero(s_lastJoystickAxis);
	Zero(s_lastJoystickSlider);

#ifdef _DEBUG
	DebugFlags::registerFlag(debugReportFlag, "SharedIoWin", "reportIoWinManager", debugReport);
	DebugFlags::registerFlag(debugReportEvents, "SharedIoWin", "reportIoWinEvents");
#endif
}

// ----------------------------------------------------------------------
/**
 * Remove the IoWinManager.
 * 
 * This routine removes the IoWinManager subststem.  It should not be called
 * normally by the game, but rather automatically through the ExitChain
 * system.
 * 
 * If there are any windows left on the window stack when this routine is
 * called, this routine will send the IOET_WindowKill event to each window
 * on the stack.  If the window processEvent() routine returns a Kill result,
 * the window will be deleted, otherwise it will just be closed.
 * 
 * @see IoWinManager::install()
 */

void IoWinManager::remove(void)
{
	IoResult   result;
	IoEvent   *event;

	Os::setQueueCharacterHookFunction(nullptr);
	Os::setSetSystemMouseCursorPositionHookFunction(nullptr);
	Os::setQueueKeyDownHookFunction(nullptr);

	DEBUG_FATAL(!installed, ("not installed"));
	installed = false;

	while (top)
	{
		// create the WindowKill event
		event = newEvent(IOET_WindowKill, 0, 0, CONST_REAL(0));
		result = top->processEvent(event);
		deleteEvent(event);

		// handle the return value
		if (isKill(result))
		{
			IoWin *currentTop = top;

			close(currentTop, false);
			delete currentTop;
		}
		else
			close(top, true);
	}

	delete eventBlockManager;

	// Remove the inactivity timer.
	registerInactivityCallback(nullptr, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Dump debugging information about the IoWinManager's stack.
 */

void IoWinManager::debugReport()
{
#ifdef _DEBUG
	IoWin *ioWin;

	DEBUG_REPORT_PRINT(true, ("IoWinManager:\n"));

	if (captured)
		DEBUG_REPORT_PRINT(true, ("  [captured] %s\n", captured->ioDebugName));

	for (ioWin = top; ioWin; ioWin = ioWin->ioNext)
		DEBUG_REPORT_PRINT(true, ("  %s\n", ioWin->ioDebugName));
#endif
}

// ----------------------------------------------------------------------
/**
 * Pass all the queued input events to the windows on the IoWinManager.
 * 
 * Events are passed down to the top window first, unless a capture window is
 * currently set, in which case the capture window will receive the event first,
 * and then it will be passed to the top window of the stack.
 * 
 * The first routine that returns a blocking result will cause the event to
 * be discarded.  If there is a capture window, and it returns a blocking result,
 * the event will not be passed down to the top window.
 */

void IoWinManager::processEvents(float elapsedTime)
{
	IoWin * w = nullptr;
	IoWin * next = nullptr;
	IoEvent * queue = nullptr;
	IoEvent * event = nullptr;

	IoResult  result;


	// setup the event list to send to the windows
	enqueueEvent(IOET_Update, 0, 0, elapsedTime);

	queue = newEvent(IOET_Prepare, 0, 0, CONST_REAL(0));
	queue->next = firstEvent;

	// the event list is now empty
	firstEvent = nullptr;
	lastEvent = nullptr;

	// Update the activity timer.
	if (!s_isInactive)
	{
		bool const isInactive = s_inactivityTimer.updateNoReset(elapsedTime);

		if (isInactive) 
		{
			// Allow the timer to trigger inactive.
			triggerInactive(true);
		}
	}

	// process all the events
	while (queue)
	{
		// remove the first element from the linked list
		event = queue;
		queue = queue->next;

		// keep people from peeking ahead in the event queue
		event->next = nullptr;


		// Check to see if the event resets 
		if (activtyDetected(event->type))
		{
			// No longer inactive.
			triggerInactive(false);

			// Reset the timer.
			s_inactivityTimer.reset();
		}

		if (discardInput)
		{
			const IoEventType type = event->type;
			if (
				type == IOET_Character ||
				type == IOET_KeyDown ||
				type == IOET_KeyUp ||
				type == IOET_JoystickMove ||
				type == IOET_JoystickButtonDown ||
				type == IOET_JoystickButtonUp ||
				type == IOET_JoystickPovHat ||
				type == IOET_JoystickSlider ||
				type == IOET_MouseMove ||
				type == IOET_MouseButtonDown ||
				type == IOET_MouseButtonUp ||
				type ==	IOET_IMEComposition ||
				type == IOET_IMEChangeCandidate ||
				type == IOET_IMECloseCandidate ||
				type == IOET_IMEEndComposition
				)
			{
				deleteEvent(event);
				continue;
			}
		}

#ifdef _DEBUG

		if (debugReportEvents)
		{
			const char *name = nullptr;

			switch (event->type)
			{
#define CASE(a) case a: name = #a; break

				CASE(IOET_WindowOpen);
				CASE(IOET_WindowClose);
				CASE(IOET_WindowKill);
				CASE(IOET_Prepare);
				CASE(IOET_Update);
				CASE(IOET_InputReset);
				CASE(IOET_Character);
				CASE(IOET_KeyDown);
				CASE(IOET_KeyUp);
				CASE(IOET_JoystickMove);
				CASE(IOET_JoystickButtonDown);
				CASE(IOET_JoystickButtonUp);
				CASE(IOET_JoystickPovHat);
				CASE(IOET_JoystickSlider);
				CASE(IOET_MouseMove);
				CASE(IOET_MouseButtonDown);
				CASE(IOET_MouseButtonUp);
				CASE(IOET_SetSystemMouseCursorPosition);
				CASE(IOET_IMEComposition);
				CASE(IOET_IMEChangeCandidate);
				CASE(IOET_IMECloseCandidate);
				CASE(IOET_IMEEndComposition);
				CASE(IOET_Count);

#undef CASE

				default:
					name = "";
					DEBUG_FATAL(true, ("default case"));
					break;
			}

			DEBUG_REPORT_PRINT(true, ("%-25s %-8d %-8d %-8.4f\n", name, event->arg1, event->arg2, event->arg3));
		}

#endif

		// If the user presses enter to close the candidate list, consume the enter key
		if (event->type == IOET_KeyDown && imeFunction)
		{
			int key = event->arg2;
			if(imeFunction(key))
			{
				deleteEvent(event);
				continue;
			}
		}

		// if there is a captured window, give it first shot at the event
		if (captured)
		{
			result = captured->processEvent(event);
			DEBUG_FATAL(isKill(result), ("captured windows can not [currently] ask to be killed"));
		}
		else
			result = IOR_Pass;

		// iterate through all the windows, passing the event down to each one
		for (result = IOR_Pass, w = top; w && isPass(result); w = next)
		{
			// save the next window
			next = w->ioNext;

			// have this window process this event
			result = w->processEvent(event);

			// check to see if the window needs to be destroyed
			if (isKill(result))
			{
				close(w, false);
				delete w;
			}
		}

		// free the event
		deleteEvent(event);
	}

	discardInput = false;
}

// ----------------------------------------------------------------------
/**
 * Draw the windows on the IoWinManager.
 * 
 * This routine will call the top window's draw routine.  That window is responsible
 * for passing along the draw message to any other windows that it wishes to allow
 * to draw.
 */

void IoWinManager::draw(void)
{
	if (top)
		top->draw();
}

// ----------------------------------------------------------------------
/**
 * Open an IoWin.
 * 
 * This places the specified window on the top of the IoWinManager stack.
 * 
 * In DEBUG compiles, this routine will verify that the window is not already on the stack.
 * If it is found to already be on the window stack, this routine will call Fatal.  This check
 * is not done in RELEASE compiles, and may corrupt the stack.
 * 
 * @param window  Window to place on top of the window stack
 * @see IoWinManager::close()
 */

void IoWinManager::open(IoWin *window)
{
	IoEvent  *event;
	IoResult  result;

	// validate the arguments
	if (!window)
	{
		DEBUG_FATAL(true, ("nullptr window"));
		return; //lint !e527 // Warning -- Unreachable
	}

#ifdef _DEBUG

	// make sure the window isn't already on the window stack
	IoWin *w;
	for (w = top; w && w != window; w = w->ioNext)
		;

	DEBUG_FATAL(w == window, ("window already open"));

#endif

	// put the window on top of the window stack
	window->ioNext = top;
	top = window;

	// send the window an open event
	event = newEvent(IOET_WindowOpen, 0, 0, CONST_REAL(0));
	result = top->processEvent(event);
	deleteEvent(event);

	// handle the window wanting to die immediately
	if (isKill(result))
	{
		close(top, false);
		delete top;
	}
}

// ----------------------------------------------------------------------
/**
 * Close a window.
 * 
 * This routine will remove the specified window from the window stack.
 * 
 * This routine will send the IOET_WindowClose message to the window.
 * If the window's processEvent() routine returns a Kill result, the
 * routine will delete the window.
 * 
 * In DEBUG compiles, if the window is not on the window stack, this routine will
 * call Fatal.  In RELEASE compiles, this routine will simply return if the window
 * is not on the window stack.
 * 
 * @param window  Window to remove from the window stack
 */

void IoWinManager::close(IoWin *window)
{
	close(window, true);
}

// ----------------------------------------------------------------------
/**
 * Close a window.
 * 
 * This routine will remove the specified window from the window stack.
 * 
 * This routine will send the IOET_WindowClose message to the window.
 * If the window's processEvent() routine returns a Kill result, the
 * routine will delete the window only if the allowDelete flag is true.
 * 
 * In DEBUG compiles, if the window is not on the window stack, this routine will
 * call Fatal.  In RELEASE compiles, this routine will simply return if the window
 * is not on the window stack.
 * 
 * @param window  Window to remove from the window stack
 * @param allowDelete  Whether to allow the routine to delete the window or not
 */

void IoWinManager::close(IoWin *window, bool allowDelete)
{
	// validate the arguments
	if (!window)
	{
		DEBUG_FATAL(true, ("nullptr window"));
		return;  //lint !e527 // Warning -- Unreachable
	}

	// linked list traversal with a back pointer
	IoWin *back, *front;
	for (back = nullptr, front = top; front && front != window; back = front, front = front->ioNext)
		;

	// verify the window was found
	if (front != window)
	{
		DEBUG_FATAL(true, ("window not found %p %s", window, window->ioDebugName));
		return;  //lint !e527 // Warning -- Unreachable
	}

	// -qq- lint hack
	DEBUG_FATAL(!window, ("nullptr window"));

	// remove it from the singly linked list
	if (back)
		back->ioNext = window->ioNext;
	else
		top = window->ioNext;

	// send the window a close event
	IoEvent *event = newEvent(IOET_WindowClose, 0, 0, CONST_REAL(0));
	const IoResult result = window->processEvent(event);
	deleteEvent(event);

	// handle the window wanting to die immediately
	if (allowDelete && isKill(result))
		delete window;
}

// ----------------------------------------------------------------------
/**
 * Allocate and fill out a new event structure.
 * 
 * This routine is used to create new events.  Events created with this
 * routine must be deleted calling deleteEvent().
 * 
 * @param eventType  Event type to be created
 * @param arg1  Event-type dependent argument value
 * @param arg2  Event-type dependent argument value
 * @param arg3  Event-type dependent argument value
 * @see IoWinManager::deleteEvent()
 */

IoEvent *IoWinManager::newEvent(IoEventType eventType, int arg1, int arg2, real arg3)
{
	// create the new event
	IoEvent * const event = reinterpret_cast<IoEvent *>(eventBlockManager->allocate());

	// fill out the event data
	event->next = nullptr;
	event->type = eventType;
	event->arg1 = arg1;
	event->arg2 = arg2;
	event->arg3 = arg3;

	return event;
}

// ----------------------------------------------------------------------
/**
 * Free an event.
 * 
 * The event must have been allocated with newEvent().
 * 
 * @see IoWinManager::newEvent()
 */

void IoWinManager::deleteEvent(IoEvent *event)
{
	eventBlockManager->free(event);
}

// ----------------------------------------------------------------------
/**
 * Create a new event and add it to the event list.
 * 
 * This routine is an internal helper routine designed to make creating a new event easier.
 * 
 * @param eventType  Event type to be created
 * @param arg1  Event-type dependent argument value
 * @param arg2  Event-type dependent argument value
 * @param arg3  Event-type dependent argument value
 */

void IoWinManager::enqueueEvent(IoEventType eventType, int arg1, int arg2, real arg3)
{
	switch (eventType)
	{
	case IOET_JoystickMove:
		if (arg1 >= 0 && arg1 < s_numJoysticks && arg2 >= 0 && arg2 < s_numJoystickAxes)
		{
			if (s_discardIdenticalJoystickMotionEvents && 
				WithinEpsilonExclusive(arg3, s_lastJoystickAxis[arg1][arg2], s_joystickAxisIdenticalThreshold))
			{
				return;
			}

			s_lastJoystickAxis[arg1][arg2] = arg3;
		}
		break;

	case IOET_JoystickSlider:
		if (arg1 >= 0 && arg1 < s_numJoysticks && arg2 >= 0 && arg2 < s_numJoystickSliders)
		{
			if (s_discardIdenticalJoystickMotionEvents && 
				WithinEpsilonExclusive(arg3, s_lastJoystickSlider[arg1][arg2], s_joystickSliderIdenticalThreshold))
			{
				return;
			}
			s_lastJoystickSlider[arg1][arg2] = arg3;
		}
		break;
	case IOET_WindowOpen:
	case IOET_WindowClose:
	case IOET_WindowKill:
	case IOET_Prepare:
	case IOET_Update:
	case IOET_InputReset:
	case IOET_Character:
	case IOET_KeyDown:
	case IOET_KeyUp:
	case IOET_JoystickButtonDown:
	case IOET_JoystickButtonUp:
	case IOET_JoystickPovHat:
	case IOET_MouseMove:
	case IOET_MouseButtonDown:
	case IOET_MouseButtonUp:
	case IOET_SetSystemMouseCursorPosition:
	case IOET_Count:
	case IOET_IMEComposition:
	case IOET_IMEChangeCandidate:
	case IOET_IMECloseCandidate:
	case IOET_IMEEndComposition:
		break;
	}

	// create the new event
	IoEvent * const event = newEvent(eventType, arg1, arg2, arg3);

	// put on the singly linked list
	if (!firstEvent)
		firstEvent = event;
	else
		lastEvent->next = event;
	lastEvent = event;
}

//----------------------------------------------------------------------

void IoWinManager::setDiscardIdenticalJoystickMotionEvents(bool b)
{
	s_discardIdenticalJoystickMotionEvents = b;
}

//----------------------------------------------------------------------

bool IoWinManager::isDiscardIdenticalJoystickMotionEvents()
{
	return s_discardIdenticalJoystickMotionEvents;
}

//----------------------------------------------------------------------

void IoWinManager::registerIMEFunction(IMEFunctionPointer imeFunctionIn)
{
	DEBUG_FATAL(imeFunction, ("IoWinManager: 2 IME functions registered"));
	imeFunction = imeFunctionIn;
}


//----------------------------------------------------------------------

void IoWinManager::registerInactivityCallback(InactivityCallback callback, float timeInSeconds = 1.0f)
{
	if (s_inactivityCallback != callback)
	{		
		// Set the activity timer.
		if (timeInSeconds > 0.0f) 
		{
			s_inactivityTimer.setExpireTime(timeInSeconds);
		}

		s_inactivityTimer.reset();
		
		// Set the new callback.
		s_inactivityCallback = callback;
	}
}

//----------------------------------------------------------------------

bool IoWinManager::isInactive(void)
{
	return s_isInactive;
}


// ======================================================================
