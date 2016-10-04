// ======================================================================
//
// DebugMonitor.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DebugMonitor.h"

#if PRODUCTION == 0

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ConfigFile.h"

// ======================================================================

namespace DebugMonitorNamespace
{
	typedef void (*ChangedWindowCallback)(int x, int y, int width, int height);

	typedef bool (*InstallFunction)(int x, int y, int width, int height);
	typedef void (*RemoveFunction)();
	typedef void (*ShowFunction)();
	typedef void (*HideFunction)();
	typedef void (*SetChangedWindowCallback)(ChangedWindowCallback);
	typedef void (*SetBehindWindowFunction)(HWND window);
	typedef void (*ClearScreenFunction)();
	typedef void (*ClearToCursorFunction)();
	typedef void (*GotoXYFunction)(int x, int y);
	typedef void (*PrintFunction)(const char *string);

	void changedWindowCallback(int x, int y, int width, int height);

	HINSTANCE               dll;
	HKEY                    registryKey = HKEY_CLASSES_ROOT;
	RemoveFunction          removeFunction;
	ShowFunction            showFunction;
	HideFunction            hideFunction;
	SetBehindWindowFunction setBehindWindowFunction;
	ClearScreenFunction     clearScreenFunction;
	ClearToCursorFunction   clearToCursorFunction;
	GotoXYFunction          gotoXYFunction;
	PrintFunction           printFunction;

	bool                    noClear;

	int GetRegistryValue(char const * name, int defaultValue)
	{
		int value;
		DWORD type = 0;
		DWORD size = sizeof(DWORD);
		LONG result = RegQueryValueEx(registryKey, name, NULL, &type, reinterpret_cast<LPBYTE>(&value), &size);
		if ((result != ERROR_SUCCESS || type != REG_DWORD) && (size != sizeof(int)))
			value = defaultValue;
		return value;
	}

	void SetRegistryValue(char const * name, int value)
	{
		RegSetValueEx(registryKey, name, NULL, REG_DWORD, reinterpret_cast<const LPBYTE>(&value), sizeof(int));
	}
}
using namespace DebugMonitorNamespace;

// ======================================================================
// Install the debug monitor subsystem
//
// Remarks:
//
//   This routine will first attempt to install the selected debug monitor.

void DebugMonitor::install()
{
	dll = LoadLibrary("debugWindow.dll");
	if (dll)
	{
		InstallFunction installFunction = reinterpret_cast<InstallFunction>(GetProcAddress(dll, "install"));
		
		RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Sony Online Entertainment\\DebugWindow", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &registryKey, NULL);

		int const x = ConfigFile::getKeyInt("SharedDebug", "debugWindowX", GetRegistryValue("x", 0));
		int const y = ConfigFile::getKeyInt("SharedDebug", "debugWindowY", GetRegistryValue("y", 0));
		int const width = ConfigFile::getKeyInt("SharedDebug", "debugWindowWidth", GetRegistryValue("width", 80));
		int const height = ConfigFile::getKeyInt("SharedDebug", "debugWindowHeight", GetRegistryValue("height", 50));

		if (installFunction && installFunction(x, y, width, height))
		{
			showFunction            = reinterpret_cast<ShowFunction>(GetProcAddress(dll, "showWindow"));
			hideFunction            = reinterpret_cast<ShowFunction>(GetProcAddress(dll, "hideWindow"));
			removeFunction          = reinterpret_cast<RemoveFunction>(GetProcAddress(dll, "remove"));
			setBehindWindowFunction = reinterpret_cast<SetBehindWindowFunction>(GetProcAddress(dll, "setBehindWindow"));
			clearScreenFunction     = reinterpret_cast<ClearScreenFunction>(GetProcAddress(dll, "clearScreen"));
			clearToCursorFunction   = reinterpret_cast<ClearToCursorFunction>(GetProcAddress(dll, "clearToCursor"));
			gotoXYFunction          = reinterpret_cast<GotoXYFunction>(GetProcAddress(dll, "gotoXY"));
			printFunction           = reinterpret_cast<PrintFunction>(GetProcAddress(dll, "print"));

			SetChangedWindowCallback setChangedWindowCallback = reinterpret_cast<SetChangedWindowCallback>(GetProcAddress(dll, "setChangedWindowCallback"));
			if (setChangedWindowCallback)
				(*setChangedWindowCallback)(changedWindowCallback);

			DebugFlags::registerFlag(noClear, "SharedDebug", "noDebugMonitorClear");

			if (ConfigFile::getKeyBool("SharedDebug", "debugWindow", false))
				show();
		}
		else
		{
			installFunction = NULL;
			const BOOL result = FreeLibrary(dll);
			dll = NULL;
			UNREF(result);
			DEBUG_FATAL(!result, ("FreeLibrary failed"));
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Remove the debug monitor subsystem.
 */

void DebugMonitor::remove()
{
	if (removeFunction)
		removeFunction();

	removeFunction = NULL;
	setBehindWindowFunction = NULL;
	clearScreenFunction = NULL;
	clearToCursorFunction = NULL;
	gotoXYFunction = NULL;
	printFunction = NULL;

	if (dll)
	{
		const BOOL result = FreeLibrary(dll);
		UNREF(result);
		DEBUG_FATAL(!result, ("FreeLibrary failed"));
		dll = NULL;

		if (registryKey != HKEY_CLASSES_ROOT)
		{
			RegCloseKey(registryKey);
			registryKey = HKEY_CLASSES_ROOT;
		}
	}
}

// ----------------------------------------------------------------------

void DebugMonitorNamespace::changedWindowCallback(int const x, int const y, int const width, int const height)
{
	SetRegistryValue("x", x);
	SetRegistryValue("y", y);
	SetRegistryValue("width", width);
	SetRegistryValue("height", height);
}

// ----------------------------------------------------------------------

void DebugMonitor::show()
{
	if (showFunction)
		(*showFunction)();
}

// ----------------------------------------------------------------------

void DebugMonitor::hide()
{
	if (hideFunction)
		(*hideFunction)();
}

// ----------------------------------------------------------------------
/**
 * Set the debug window's z-order.
 */

void DebugMonitor::setBehindWindow(HWND window)
{
	if (setBehindWindowFunction)
		setBehindWindowFunction(window);
}

// ----------------------------------------------------------------------
/**
 * Clear the debug monitor and home the cursor.
 * 
 * If the mono monitor is not installed, this routine does nothing.
 * 
 * This routine will clear the contents of the debug monitor, reset the screen
 * offset to 0, and move the cursor to the upper left corner of the screen.
 * 
 * @see DebugMonitor::home(), DebugMonitor::clearToCursor()
 */

void DebugMonitor::clearScreen()
{
	if (noClear)
		return;

	if (clearScreenFunction)
		clearScreenFunction();
}

// ----------------------------------------------------------------------
/**
 * Clear the debug monitor to the current cursor position and home the cursor.
 * 
 * If the debug monitor is not installed, this routine does nothing.
 * 
 * This routine will clear the contents of the debug monitor only up to the
 * cursor position.  If the cursor is not very far down on the screen,
 * this routine may be significantly more efficient clearing the screen.
 * 
 * It will also move the cursor to the upper left corner of the screen.
 * 
 * @see DebugMonitor::clearScreen(), DebugMonitor::home()
 */

void DebugMonitor::clearToCursor()
{
	if (clearToCursorFunction)
		clearToCursorFunction();
	else
		clearScreen();
}

// ======================================================================
// Move the cursor to the upper left hand corner of the mono monitor screen
//
// Remarks:
//
//   If the debug monitor is not installed, this routine does nothing.
//
//   All printing happens at the cursor position.
//
//   This routine is identical to calling gotoXY(0,0);
//
// See Also:
//
//   DebugMonitor::gotoXY()

void DebugMonitor::home()
{
	gotoXY(0,0);
}

// ----------------------------------------------------------------------
/**
 * Position the cursor on the debug monitor screen.
 * 
 * If the debug monitor is not installed, this routine does nothing.
 * 
 * All printing happens at the cursor position.
 * 
 * @param x  New X position for the cursor
 * @param y  New Y position for the cursor
 */

void DebugMonitor::gotoXY(int x, int y)
{
	if (gotoXYFunction)
		gotoXYFunction(x, y);
}

// ----------------------------------------------------------------------
/**
 * Display a string on the debug monitor.
 * 
 * If the debug monitor is not installed, this routine does nothing.
 * 
 * Printing occurs from the cursor position.
 * 
 * Newline characters '\n' will cause the cursor position to advance to the
 * beginning of the next line.  If the cursor is already on the last line of
 * the screen, the screen will scroll up one line and the cursor will move to
 * the beginning of the last line.
 * 
 * The backspace character '\b' will cause the cursor to move one character
 * backwards.  If at the beginning of the line, the cursor will move to the
 * end of the previous line.  If already on the first line of the screen, the
 * cursor position and screen contents will be unchanged.
 * 
 * All other characters are placed directly into the text frame buffer.
 * After each character, the cursor will be logically advanced one
 * character forward.  If the cursor was on the last column, it will advance
 * to the next line.  If the cursor was already on the last line, the screen
 * will be scrolled up one line and the cursor will move to the beginning of
 * the last line.
 * 
 * @param string  String to display on the debug monitor
 */

void DebugMonitor::print(const char *string)
{
	if (printFunction)
		printFunction(string);
}

// ----------------------------------------------------------------------
/**
 * Ensure all changes to the DebugMonitor have taken effect by the time
 * this function returns.
 *
 * Note: some platforms may do nothing here.  The Win32 platform does not
 *       require flushing.  The Linux platform does.  Call it assuming
 *       that it is needed.  It will be a no-op when not required.
 */

void DebugMonitor::flushOutput()
{
	// Win32 debug monitors don't need to do anything here.
}

// ======================================================================

#endif
