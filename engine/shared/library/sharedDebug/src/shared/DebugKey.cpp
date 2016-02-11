// ======================================================================
//
// DebugKey.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DebugKey.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Production.h"

#include <algorithm>
#include <vector>

// ======================================================================

#if PRODUCTION == 0
bool                  *DebugKey::ms_currentFlag;
bool                   DebugKey::ms_pressed[10];
bool                   DebugKey::ms_down[10];
DebugKey::FlagVector   DebugKey::ms_flags;
#endif

// ======================================================================
/**
 * @internal
 */

bool DebugKey::Flag::operator <(const Flag &rhs) const
{
	return strcmp(name, rhs.name) < 0;
}

// ----------------------------------------------------------------------

void DebugKey::newFrame()
{
#if PRODUCTION == 0
	memset(ms_pressed, 0, sizeof(ms_pressed));
#endif
}

// ----------------------------------------------------------------------

void DebugKey::lostFocus()
{
#if PRODUCTION == 0
	memset(ms_down, 0, sizeof(ms_down));
	memset(ms_pressed, 0, sizeof(ms_pressed));
#endif
}

// ----------------------------------------------------------------------
/**
 * Register a debug key flag.
 * The parameters passed to this function must never go out of scope.
 * @param variable The bool variable to determine whether the flag is on or not.
 * @param name     The name of the variable.
 */

void DebugKey::registerFlag(bool &variable, const char *name)
{
#if PRODUCTION == 0
	if (ConfigFile::isInstalled())
		variable = ConfigFile::getKeyBool("SharedDebug/DebugKey", name, variable);

	Flag f;
	f.variable = &variable;
	f.name = name;
	ms_flags.insert(std::lower_bound(ms_flags.begin(), ms_flags.end(), f), f);
#else
	UNREF(variable);
	UNREF(name);
#endif
}

// ----------------------------------------------------------------------

bool DebugKey::isPressed(int i)
{
#if PRODUCTION == 0
	DEBUG_FATAL(i < 0 || i > 9, ("DebugKey::isPressed out of range 0/%d/9", i));
	return ms_pressed[i];
#else
	UNREF(i);
	return false;
#endif
}

// ----------------------------------------------------------------------

bool DebugKey::isDown(int i)
{
#if PRODUCTION == 0
	DEBUG_FATAL(i < 0 || i > 9, ("DebugKey::isDown out of range 0/%d/9", i));
	return (ms_pressed[i] || ms_down[i]);
#else
	UNREF(i);
	return false;
#endif
}

// ----------------------------------------------------------------------

bool DebugKey::isActive()
{
#if PRODUCTION == 0
	return ms_currentFlag != nullptr;
#else
	return false;
#endif
}

// ----------------------------------------------------------------------

void DebugKey::setCurrentFlag(const bool *newFlag)
{
#if PRODUCTION == 0
	lostFocus();

	// make sure the old flag gets set false
	if (ms_currentFlag)
		*ms_currentFlag = false;

	// update the flag pointer
	ms_currentFlag = const_cast<bool *>(newFlag);

	// make sure the new flag gets set true
	if (ms_currentFlag)
		*ms_currentFlag = true;
#else
	UNREF(newFlag);
#endif
}

// ----------------------------------------------------------------------

void DebugKey::pressKey(int i)
{
#if PRODUCTION == 0
	DEBUG_FATAL(i < 0 || i > 9, ("DebugKey::pressKey out of range 0/%d/9", i));
	ms_pressed[i] = true;
	ms_down[i] = true;
#else
	UNREF(i);
#endif
}

// ----------------------------------------------------------------------

void DebugKey::releaseKey(int i)
{
#if PRODUCTION == 0
	DEBUG_FATAL(i < 0 || i > 9, ("DebugKey::releastKey out of range 0/%d/9", i));
	ms_down[i] = false;
#else
	UNREF(i);
#endif
}

// ======================================================================
