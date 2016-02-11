// ======================================================================
//
// IoWinManager.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_IoWinManager_H
#define INCLUDED_IoWinManager_H

// ======================================================================

#include "sharedIoWin/IoWin.def"
class IoWin;
class MemoryBlockManager;

// ======================================================================

class IoWinManager
{
	typedef bool (*IMEFunctionPointer)(int key);
public:

	static bool                              installed;
	static bool                              discardInput;

#ifdef _DEBUG
	static bool                              debugReportFlag;
	static bool                              debugReportEvents;
#endif

	static IoWin                            *top;
	static IoWin                            *captured;

	static IoEvent                          *firstEvent;
	static IoEvent                          *lastEvent;

	static MemoryBlockManager *eventBlockManager;

	static IMEFunctionPointer               imeFunction;

private:
	static IoEvent *newEvent(IoEventType eventType, int arg1, int arg2, real arg3);
	static void     deleteEvent(IoEvent *event);
	static void     enqueueEvent(IoEventType eventType, int arg1, int arg2, real arg3);
	static void     close(IoWin *window, bool allowDelete);

protected:

	// disabled
	IoWinManager(void);
	IoWinManager(const IoWinManager &);
	IoWinManager &operator =(const IoWinManager &);

public:

	static void install();
	static void remove(void);

	static void debugReport();

	static void processEvents(float elapsedTime);
	static void draw(void);

	static bool isPass(IoResult result);
	static bool isBlock(IoResult result);
	static bool isKill(IoResult result);

	static bool haveWindow(void);

	static void open(IoWin *window);
	static void close(IoWin *window);

	static void discardUserInputUntilNextProcessEvents();
	static void setDiscardIdenticalJoystickMotionEvents(bool b);
	static bool isDiscardIdenticalJoystickMotionEvents();
	
	static void queueInputReset(void);
	static void queueCharacter(int keyboard, int character);
	static void queueKeyDown(int keyboard, int key);
	static void queueKeyUp(int keyboard, int key);

	static void queueMouseButtonDown(int mouse, int mouseButton);
	static void queueMouseButtonUp(int mouse, int mouseButton);

	static void queueMouseTranslateX(int mouse, real value);
	static void queueMouseTranslateY(int mouse, real value);
	static void queueMouseTranslateZ(int mouse, real value);
	static void queueMouseRotateX(int mouse, real value);
	static void queueMouseRotateY(int mouse, real value);
	static void queueMouseRotateZ(int mouse, real value);

	static void queueJoystickTranslateX(int joystick, real value);
	static void queueJoystickTranslateY(int joystick, real value);
	static void queueJoystickTranslateZ(int joystick, real value);
	static void queueJoystickRotateX(int joystick, real value);
	static void queueJoystickRotateY(int joystick, real value);
	static void queueJoystickRotateZ(int joystick, real value);
	static void queueJoystickPOVHatCentered(int joystick, int hat);
	static void queueJoystickPOVHatOffset(int joystick, int hat, real value);
	static void queueJoystickButtonDown(int joystick, int mouseButton);
	static void queueJoystickButtonUp(int joystick, int mouseButton);
	static void queueJoystickSlider(int joystick, int slider, real value);

	static void queueSetSystemMouseCursorPosition(int x, int y);

	static void queueIMEComposition(void);
	static void queueIMEChangeCandidate(void);
	static void queueIMECloseCandidate(void);
	static void queueIMEEndComposition(void);

	static void registerIMEFunction(IMEFunctionPointer imeFunctionIn);

	typedef bool (*InactivityCallback)(bool inactive);
	static void registerInactivityCallback(InactivityCallback callback, float timeInSeconds);
	static bool isInactive(void);
};

// ======================================================================
// Determine if any windows are currently on the window stack
//
// Return value:
//
//   True if any windows are on the IoWinManager's stack, false otherwise
//
// Remarks:
//
//   The game may want to terminate if no windows are open on the IoWinManager.

inline bool IoWinManager::haveWindow(void)
{
	return (top != nullptr);
}

// ----------------------------------------------------------------------
/** Discard user input.
 *
 * This routine will cause the next call to processEvents() to ignore all
 * user input events (keyboard, mouse, and joystick).
 */

inline void IoWinManager::discardUserInputUntilNextProcessEvents()
{
	discardInput = true;
}

// ----------------------------------------------------------------------
/**
 * Enqueue a character press.
 * 
 * This adds a input-reset event to the queue.  That event indicates that
 * the input devices are now in unknown state and any previous state is
 * invalid.
 * 
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueInputReset(void)
{
	enqueueEvent(IOET_InputReset, 0, 0, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a character press.
 * 
 * This adds a typeable character to the input queue.  The character should already
 * have any necessary shifting applied to it.  The characters should be read for
 * text input, not for key presses.
 * 
 * @param keyboard  Keyboard that generated the character
 * @param character  Character to be enqueued
 * @return Pointer to the event that was enqueued
 * @see IoWinManager::queueKeyPress()
 */

inline void IoWinManager::queueCharacter(int keyboard, int character)
{
	enqueueEvent(IOET_Character, keyboard, character, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a key down.
 * 
 * This adds a keyboard key press to the input queue.  Events of this type should
 * not be used for string entry.
 * 
 * @param keyboard  Keyboard that generated the character
 * @param key  Key that was pressed
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueKeyDown(int keyboard, int key)
{
	enqueueEvent(IOET_KeyDown, keyboard, key, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a key up message.
 * 
 * This adds a keyboard key release to the input queue.
 * 
 * @param keyboard  Keyboard that generated the character
 * @param key  Key that was released
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueKeyUp(int keyboard, int key)
{
	enqueueEvent(IOET_KeyUp, keyboard, key, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse button down message.
 * 
 * This adds a mouse button down to the input queue.
 * 
 * @param mouse  Mouse that generated this event
 * @param mouseButton  Mouse button that was pressed
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseButtonDown(int mouse, int mouseButton)
{
	enqueueEvent(IOET_MouseButtonDown, mouse, mouseButton, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse button up message.
 * 
 * This adds a mouse button up to the input queue.
 * 
 * @param mouse  Mouse that generated this event
 * @param mouseButton  Mouse button that was released
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseButtonUp(int mouse, int mouseButton)
{
	enqueueEvent(IOET_MouseButtonUp, mouse, mouseButton, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse x translation message.
 * 
 * This adds a message indicating that the mouse moved in the X direction.
 * 
 * @param mouse  Mouse that generated this event
 * @param offset  Distance the mouse moved in the X direction
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseTranslateX(int mouse, real offset)
{
	enqueueEvent(IOET_MouseMove, mouse, IOMT_TranslateX, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse y translation message.
 * 
 * This adds a message indicating that the mouse moved in the Y direction.
 * 
 * @param mouse  Mouse that generated this event
 * @param offset  Distance the mouse moved in the Y direction
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseTranslateY(int mouse, real offset)
{
	enqueueEvent(IOET_MouseMove, mouse, IOMT_TranslateY, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse z translation message.
 * 
 * This adds a message indicating that the mouse moved in the Z direction.
 * 
 * @param mouse  Mouse that generated this event
 * @param offset  Distance the mouse moved in the Z direction
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseTranslateZ(int mouse, real offset)
{
	enqueueEvent(IOET_MouseMove, mouse, IOMT_TranslateZ, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse x rotation message.
 * 
 * This adds a message indicating that the mouse rotated around the X axis
 * 
 * @param mouse  Mouse that generated this event
 * @param offset  Amount the mouse rotated around the X axis
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseRotateX(int mouse, real offset)
{
	enqueueEvent(IOET_MouseMove, mouse, IOMT_RotateX, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse y rotation message.
 * 
 * This adds a message indicating that the mouse rotated around the YX axis
 * 
 * @param mouse  Mouse that generated this event
 * @param offset  Amount the mouse rotated around the Y axis
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseRotateY(int mouse, real offset)
{
	enqueueEvent(IOET_MouseMove, mouse, IOMT_RotateY, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a mouse z rotation message.
 * 
 * This adds a message indicating that the mouse rotated around the Z axis
 * 
 * @param mouse  Mouse that generated this event
 * @param offset  Amount the mouse rotated around the Y axis
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueMouseRotateZ(int mouse, real offset)
{
	enqueueEvent(IOET_MouseMove, mouse, IOMT_RotateZ, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick x translation message.
 * 
 * This adds a message indicating that the joystick offset in the X direction
 * 
 * @param joystick  Joystick that generated this event
 * @param offset  Joystick offset in the X direction
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickTranslateX(int joystick, real offset)
{
	enqueueEvent(IOET_JoystickMove, joystick, IOMT_TranslateX, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick y translation message.
 * 
 * This adds a message indicating that the joystick offset in the Y direction
 * 
 * @param joystick  Joystick that generated this event
 * @param offset  Joystick offset in the Y direction
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickTranslateY(int joystick, real offset)
{
	enqueueEvent(IOET_JoystickMove, joystick, IOMT_TranslateY, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick z translation message.
 * 
 * This adds a message indicating that the joystick offset in the Z direction
 * 
 * @param joystick  Joystick that generated this event
 * @param offset  Joystick offset in the Z direction  
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickTranslateZ(int joystick, real offset)
{
	enqueueEvent(IOET_JoystickMove, joystick, IOMT_TranslateZ, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick x rotation message.
 * 
 * This adds a message indicating that the joystick offset around the X axis
 * 
 * @param joystick  Joystick that generated this event
 * @param offset  Joystick offset around the X axis
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickRotateX(int joystick, real offset)
{
	enqueueEvent(IOET_JoystickMove, joystick, IOMT_RotateX, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick y rotation message.
 * 
 * This adds a message indicating that the joystick offset around the Y axis
 * 
 * @param joystick  Joystick that generated this event
 * @param offset  Joystick offset around the Y axis
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickRotateY(int joystick, real offset)
{
	enqueueEvent(IOET_JoystickMove, joystick, IOMT_RotateY, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick z rotation message.
 * 
 * This adds a message indicating that the joystick offset around the Z axis
 * 
 * @param joystick  Joystick that generated this event
 * @param offset  Joystick offset around the Z axis
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickRotateZ(int joystick, real offset)
{
	enqueueEvent(IOET_JoystickMove, joystick, IOMT_RotateZ, offset);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a POV hat centered message.
 * 
 * This adds a message indicating that the specified POV hat is centered.
 * The offset for a centered hat is -1.
 * 
 * @param joystick  Joystick that generated this event
 * @param hat  POV hat that is centered
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickPOVHatCentered(int joystick, int hat)
{
	enqueueEvent(IOET_JoystickPovHat, joystick, hat, CONST_REAL(-1));
}

// ----------------------------------------------------------------------
/**
 * Enqueue a POV hat offset message.
 * 
 * This adds a message indicating that the specified POV hat is offset.
 * The offset goes from 0 to 2*Pi.
 * 
 * @param joystick  Joystick that generated this event
 * @param hat  POV hat that is offset
 * @param value  Offset from 0 to 2*Pi
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickPOVHatOffset(int joystick, int hat, real value)
{
	enqueueEvent(IOET_JoystickPovHat, joystick, hat, value);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick button down message.
 * 
 * This adds a message indicating that the specified joystick button was
 * pressed.
 * 
 * @param joystick  Joystick that generated this event
 * @param joystickButton  Joystick button that was pressed
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickButtonDown(int joystick, int joystickButton)
{
	enqueueEvent(IOET_JoystickButtonDown, joystick, joystickButton, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick button up message.
 * 
 * This adds a message indicating that the specified joystick button was
 * released.
 * 
 * @param joystick  Joystick that generated this event
 * @param joystickButton  Joystick button that was released
 * @return Pointer to the event that was enqueued
 */

inline void IoWinManager::queueJoystickButtonUp(int joystick, int joystickButton)
{
	enqueueEvent(IOET_JoystickButtonUp, joystick, joystickButton, 0.0f);
}

// ----------------------------------------------------------------------

inline void IoWinManager::queueJoystickSlider(int joystick, int slider, real value)
{
	enqueueEvent(IOET_JoystickSlider, joystick, slider, value);
}

// ----------------------------------------------------------------------

inline void IoWinManager::queueSetSystemMouseCursorPosition(int x, int y)
{
	enqueueEvent(IOET_SetSystemMouseCursorPosition, x, y, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue an IME Composition message
 * 
 * This adds a message indicating that the current IME composition has changed
 * 
 */

inline void IoWinManager::queueIMEComposition(void)
{
	enqueueEvent(IOET_IMEComposition, 0, 0, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue an IME Change Candidate message
 * 
 * This adds a message indicating that the candidate list has changed
 * 
 */

inline void IoWinManager::queueIMEChangeCandidate(void)
{
	enqueueEvent(IOET_IMEChangeCandidate, 0, 0, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue an IME Close Candidate message
 * 
 * This adds a message indicating that candidate list has been closed
 * 
 */

inline void IoWinManager::queueIMECloseCandidate(void)
{
	enqueueEvent(IOET_IMECloseCandidate, 0, 0, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Enqueue a joystick button down message.
 * 
 * This adds a message indicating that the specified joystick button was
 * pressed.
 * 
 * @param joystick  Joystick that generated this event
 * @param joystickButton  Joystick button that was pressed
 * @return Pointer to the event that was enqueued
 */
		
inline void IoWinManager::queueIMEEndComposition(void)
{
	enqueueEvent(IOET_IMEEndComposition, 0, 0, 0.0f);
}

// ----------------------------------------------------------------------
/**
 * Determine if a IoResult is a pass result.
 * 
 * This routine tests a single bit in the result to check if it is a pass value.
 * 
 * @return True if the result is IOR_Pass or IOR_PassKillMe, false otherwise
 * @see IoWinManager::isBlock(), IoWinManager::isKill()
 */

inline bool IoWinManager::isPass(IoResult result)
{
	return (static_cast<int>(result) & IOR_PASS_BIT) != 0;
}

// ----------------------------------------------------------------------
/**
 * Determine if a IoResult is a block result.
 * 
 * This routine tests a single bit in the result to check if it is a block value.
 * 
 * @return True if the result is IOR_Block or IOR_BlockKillMe, false otherwise
 * @see IoWinManager::isPass(), IoWinManager::isKill()
 */

inline bool IoWinManager::isBlock(IoResult result)
{
	return (static_cast<int>(result) & IOR_BLOCK_BIT) != 0;
}

// ----------------------------------------------------------------------
/**
 * Determine if a IoResult is a kill result.
 * 
 * This routine tests a single bit in the result to check if it is a kill value.
 * 
 * @return True if the result is IOR_PassKillMe or IOR_BlockKillMe, false otherwise
 * @see IoWinManager::isPass(), IoWinManager::isBlock()
 */

inline bool IoWinManager::isKill(IoResult result)
{
	return (static_cast<int>(result) & IOR_KILL_BIT) != 0;
}

// ======================================================================

#endif

