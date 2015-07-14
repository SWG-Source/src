// ======================================================================
//
// Os.cpp
//
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Os.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugKey.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FloatingPointUnit.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/StringCompare.h"
#include "sharedFoundation/WindowsWrapper.h"
#include "shellapi.h"

#include <stack>
#include <map>
#include <string>
#include <vector>

// ======================================================================

namespace OsNamespace
{
	void   applyWindowChanges();
	void   updateMousePosition(int x, int y);

	const int PROGRAM_NAME_SIZE = 512;

	bool                                          ms_installed;
	bool                                          ms_processMessagePump = true;

#if PRODUCTION == 0
	bool                                          ms_validateGuardPatterns;
	bool                                          ms_validateFreePatterns;
	bool                                          ms_allowPopupDebugMenu;
#endif

	int                                           ms_numberOfUpdates;
	HWND                                          ms_window;
	HCURSOR                                       ms_cursorArrow;
	bool                                          ms_engineOwnsWindow;
	bool                                          ms_wasFocusLost;
	bool                                          ms_gameOver;
	bool                                          ms_shouldReturnFromAbort;
	bool                                          ms_wantPopupDebugMenu;
	bool                                          ms_threadDied;
	bool                                          ms_mouseMoveInClient;
	bool                                          ms_clickToMove;
	char                                          ms_programName[PROGRAM_NAME_SIZE];
	char                                         *ms_shortProgramName;
	char                                          ms_programStartupDirectory[MAX_PATH];
	Os::ThreadId                                  ms_mainThreadId;
	Os::IsGdiVisibleHookFunction                  ms_isGdiVisibleHookFunction;
	Os::LostFocusHookFunction                     ms_lostFocusHookFunction;
	Os::AcquiredFocusHookFunction                 ms_acquiredFocusHookFunction;
	Os::AcquiredFocusHookFunction                 ms_acquiredFocusHookFunction2;
	Os::QueueCharacterHookFunction                ms_queueCharacterHookFunction;
	Os::SetSystemMouseCursorPositionHookFunction  ms_setSystemMouseCursorPositionHookFunction;
	Os::SetSystemMouseCursorPositionHookFunction  ms_setSystemMouseCursorPositionHookFunction2;
	Os::GetHardwareMouseCursorEnabled             ms_getHardwareMouseCursorEnabled;
	Os::GetOtherAdapterRectsHookFunction          ms_getOtherAdapterRectsHookFunction;
	Os::WindowPositionChangedHookFunction         ms_windowPositionChangedHookFunction;
	Os::DisplayModeChangedHookFunction            ms_displayModeChangedHookFunction;
	Os::InputLanguageChangedHookFunction          ms_inputLanguageChangedHookFunction;
	Os::IMEHookFunction                           ms_IMEHookFunction;
	Os::QueueKeyDownHookFunction                  ms_queueKeyDownHookFunction;

	int                                           ms_processorCount;
	int                                           ms_debugKeyIndex;
	int                                           ms_SystemMouseCursorPositionX;
	int                                           ms_SystemMouseCursorPositionY;

	std::vector<RECT>                             ms_otherAdapterRects;

	char                                          ms_keyboardLayout[KL_NAMELENGTH];

	bool                                          ms_focused;

	int const                                     ms_hotKeyId = 0xBEEF;

	extern "C" WINBASEAPI BOOL WINAPI IsDebuggerPresent(VOID);
};

using namespace OsNamespace;


static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ======================================================================
/**
 * Install the Os subsystem for games
 * 
 * This routine is supported for all platforms, although different platforms may
 * require different arguments to this routine.
 * 
 * This routine registers the window class and creates the window for the application.
 * 
 * This routine will add Os::remove to the ExitChain.
 * 
 * @param instance    Handle to the instance for this application
 * @param windowName  Name for the window title bar
 * @paran normalIcon  Normal icon for the game
 * @param smallIcon   Small icon for the task bar
 * @see Os::remove()
 */

void Os::install(HINSTANCE instance, const char *windowName, HICON normalIcon, HICON smallIcon)
{
	installCommon();

	// setup the window class
	WNDCLASSEX  wclass;
	Zero(wclass);
	wclass.cbSize        = sizeof(wclass);
	wclass.style         = CS_BYTEALIGNCLIENT;
	wclass.lpfnWndProc   = WindowProc;
	wclass.hInstance     = instance;
	wclass.hIcon         = normalIcon;
	wclass.hCursor       = NULL;
	wclass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wclass.lpszClassName = windowName;
	wclass.hIconSm       = smallIcon;

	// register the window class
	ATOM atom = RegisterClassEx(&wclass);
	FATAL(atom == 0, ("RegisterClassEx failed"));

	// create the window
	ms_window = CreateWindow(
		windowName,                                 // pointer to registered class name
		windowName,                                 // pointer to window name
		WS_POPUP,                                   // window style
		0,                                          // horizontal position of window
		0,                                          // vertical position of window
		640,                                        // window width
		480,                                        // window height
		NULL,                                       // handle to parent or owner window
		NULL,                                       // handle to menu or child-window identifier
		instance,                                   // handle to application instance
		NULL);                                      // pointer to window-creation data
	FATAL(!ms_window, ("CreateWindow failed"));
	ms_engineOwnsWindow = true;

	// load the arrow cursor
	ms_cursorArrow = LoadCursor(NULL, IDC_ARROW);
	FATAL(ms_cursorArrow == NULL, ("LoadCursor failed"));
}

// ----------------------------------------------------------------------
/**
 * Install the Os subsystem for non-games.
 * 
 * This routine is supported for all platforms, although different platforms may
 * require different arguments to this routine.F
 * 
 * This routine will add Os::remove to the ExitChain.
 * 
 * @see Os::remove()
 */

void Os::install(HWND newWindow, bool newProcessMessagePump)
{
	installCommon();
	ms_window = newWindow;
	ms_processMessagePump = newProcessMessagePump;
}

// ----------------------------------------------------------------------
/**
 * Install the Os subsystem for non-games.
 * 
 * This routine is supported for all platforms, although different platforms may
 * require different arguments to this routine.
 * 
 * This routine will add Os::remove to the ExitChain.
 * 
 * @see Os::remove()
 */

void Os::install()
{
	installCommon();
}

// ----------------------------------------------------------------------
/**
 * This routine will remove the Os subsystem.
 * 
 * This routine should not be called directly.  It will be called from the ExitChain.
 * 
 * @see Os::install()
 */

void Os::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	ms_installed = false;
}

// ----------------------------------------------------------------------

void Os::installCommon()
{
	DEBUG_FATAL(ms_installed, ("already installed"));

	ExitChain::add(Os::remove, "Os::remove", 0, true);

	// get startup folder.
	GetCurrentDirectory(sizeof(ms_programStartupDirectory), ms_programStartupDirectory);

	ms_numberOfUpdates = 0;
	ms_mainThreadId = GetCurrentThreadId();
	setThreadName(ms_mainThreadId, "Main");

#if PRODUCTION == 0
	ms_allowPopupDebugMenu = ConfigFile::getKeyBool("SharedFoundation", "allowPopupDebugMenu", false);
#endif

	// get the name of the executable
	DWORD result = GetModuleFileName(NULL, ms_programName, sizeof(ms_programName));
	FATAL(result == 0, ("GetModuleFileName failed"));

	// get the file name without the path
	ms_shortProgramName = strrchr(ms_programName, '\\');
	if (ms_shortProgramName)
		++ms_shortProgramName;
	else
		ms_shortProgramName = ms_programName;

	// switch into single-precision floating point mode
	FloatingPointUnit::install();

	// get the amount of memory
	MEMORYSTATUS memoryStatus;
	GlobalMemoryStatus(&memoryStatus);
	CrashReportInformation::addStaticText("Ram: %dmb\n", memoryStatus.dwTotalPhys / (1024 * 1024));

	// log the os information
	{
		OSVERSIONINFO versionInfo;
		Zero(versionInfo);
		versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&versionInfo);
		CrashReportInformation::addStaticText("Os1: %d.%d.%d\n", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion, versionInfo.dwBuildNumber);

		char const * os = "Unknown";
		if (versionInfo.dwMajorVersion == 4 && versionInfo.dwMinorVersion == 10)
			os = "Windows 98";
		else
			if (versionInfo.dwMajorVersion == 4 && versionInfo.dwMinorVersion == 90)
				os = "Windows Me";
			else
				if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 0)
					os = "Windows 2000";
				else
					if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 1)
						os = "Windows XP";
					else
						if (versionInfo.dwMajorVersion == 5 && versionInfo.dwMinorVersion == 2)
							os = "Windows Server 2003";
						else
							if (versionInfo.dwMajorVersion == 6 && versionInfo.dwMinorVersion == 0)
								os = "Windows Vista";



		CrashReportInformation::addStaticText("Os2: %s %s\n", os, versionInfo.szCSDVersion);
	}

	// get the number of processors
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	ms_processorCount = static_cast<int>(si.dwNumberOfProcessors);
	REPORT_LOG (ConfigSharedFoundation::getVerboseHardwareLogging(), ("Processor Count: %i\n", ms_processorCount));
	CrashReportInformation::addStaticText("NumProc: %d\n", ms_processorCount);

	{
		HKEY key;
		LONG result = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_EXECUTE, &key);

		if (result == ERROR_SUCCESS) 
		{
			DWORD data; 
			DWORD type = REG_DWORD;
			DWORD size = sizeof (data);
			result = RegQueryValueEx (key, "~MHz", NULL, &type, reinterpret_cast<LPBYTE> (&data), &size);
			if ((result == ERROR_SUCCESS) && (size > 0))
				REPORT_LOG (ConfigSharedFoundation::getVerboseHardwareLogging(), ("Processor Speed: %i MHz\n", data));

			RegCloseKey (key);
		}
	}

	if (!GetKeyboardLayoutName(ms_keyboardLayout))
		ms_keyboardLayout[0] = '\0';

	ms_installed = true;

#if PRODUCTION == 0
	DebugFlags::registerFlag(ms_validateGuardPatterns, "SharedFoundation", "validateGuardPatterns");
	DebugFlags::registerFlag(ms_validateFreePatterns, "SharedFoundation", "validateFreePatterns");
#endif

	switch (ConfigSharedFoundation::getProcessPriority())
	{
		case -1:
			setProcessPriority(P_low);
			break;

		case 0:
			setProcessPriority(P_normal);
			break;

		case 1:
			setProcessPriority(P_high);
			break;

		default:
			DEBUG_WARNING(true, ("invalid process priority, %d should be betweein [-1..1]", ConfigSharedFoundation::getProcessPriority()));
			break;
	}
}

// ======================================================================
// Return the window handle
//
// Return value:
//
//   Handle to the window for this application
//
// Remarks:
//
//   This routine is only supported on the Win* platforms, and should not be used by the
//   game or the engine if portability is required.

HWND Os::getWindow()
{
	return ms_window;
}

// ----------------------------------------------------------------------

bool Os::engineOwnsWindow()
{
	return ms_engineOwnsWindow;
}

// ----------------------------------------------------------------------
/**
 * Return the full name of the running executable.
 *
 * The program name will include the path as well.
 *
 * @return The full name of the running executable
 * @see Os::getShortProgramName()
 */

const char *Os::getProgramName()
{
	return ms_programName;
}

// ----------------------------------------------------------------------
/**
 * Return the short name of the running executable.
 *
 * The program name will not include the path, but will just be the file name.
 *
 * @return The short name of the running executable
 * @see Os::getProgramName()
 */

const char *Os::getShortProgramName()
{
	return ms_shortProgramName;
}

// ----------------------------------------------------------------------
/**
 * Return the current working directory when the program was started.
 *
 */

const char *Os::getProgramStartupDirectory()
{
	return ms_programStartupDirectory;
}

// ----------------------------------------------------------------------
/**
 * Cause Os::abort() to return instead of abort the process.
 *
 * This routine should not be called directly by users.
 *
 * This routine is provided so that structured exception handling can catch
 * an exception, call Fatal to run the ExitChain, and rethrow the exception
 * so that the debugger will catch it.
 */

void Os::returnFromAbort()
{
	ms_shouldReturnFromAbort = true;
}

// ----------------------------------------------------------------------
/**
 * Check if the Os knows the game needs to shut down.
 *
 * The Os can decide that the game need to end for a number of reasons,
 * including closing the application or shutting the machine down.
 *
 * @return True if the game should quit, otherwise false
 */

bool Os::isGameOver()
{
	return ms_gameOver;
}

// ----------------------------------------------------------------------
/**
 * Return the number of updates that the have occurred.
 *
 * @return This value is updated during the Os::update() routine.
 */

int Os::getNumberOfUpdates()
{
	return ms_numberOfUpdates;
}

// ----------------------------------------------------------------------
/**
 * Indicate whether or not the application was paused.
 *
 * @return True if the application was in the background (paused), otherwise false
 */

bool Os::wasFocusLost()
{
	return ms_wasFocusLost;
}

// ----------------------------------------------------------------------
/**
 * Return a flag indicating whether we are running a multiprocessor machine or not.
 *
 * @return True if the machine has more than one processor, false if not.
 */

bool Os::isMultiprocessor()
{
	return ms_processorCount > 1;
}

// ----------------------------------------------------------------------
/**
 * Return the number of processors.
 *
 * @return The number of processors in the machine.
 */

int Os::getProcessorCount()
{
	return ms_processorCount;
}

// ----------------------------------------------------------------------

bool Os::isMainThread()
{
	// if the Os class hasn't been installed, then assume we are the main thread.
	// otherwise, check to see if our thread id is the main thread id
	return !ms_installed || (GetCurrentThreadId() == ms_mainThreadId);
}

// ----------------------------------------------------------------------
/**
 * Terminate the application because of an error condition.
 * 
 * This routine is supported for all platforms.
 * 
 * This routine should not be called directly.  The engine and game should use the
 * FATAL macro to terminate the application because of an error.
 * 
 * Calling Os::returnFromAbort() will cause the routine to do nothing but return
 * immediately.
 * 
 * @see Os::returnFromAbort(), FATAL()
 */

void Os::abort()
{
	if (!isMainThread())
	{
		ms_threadDied = true;
		ExitThread(1);
	}

	if (!ms_shouldReturnFromAbort)
	{
		// let the C runtime deal with the abnormal termination
		::abort();
	}
}

// ----------------------------------------------------------------------

void Os::setLostFocusHookFunction(LostFocusHookFunction lostFocusHookFunction)
{
	ms_lostFocusHookFunction = lostFocusHookFunction;
}

// ----------------------------------------------------------------------

void Os::setIsGdiVisibleHookFunction(IsGdiVisibleHookFunction isGdiVisibleHookFunction)
{
	ms_isGdiVisibleHookFunction = isGdiVisibleHookFunction;
}

// ----------------------------------------------------------------------

void Os::setQueueCharacterHookFunction(QueueCharacterHookFunction queueCharacterHookFunction)
{
	ms_queueCharacterHookFunction = queueCharacterHookFunction;
}

// ----------------------------------------------------------------------

void Os::setSetSystemMouseCursorPositionHookFunction(SetSystemMouseCursorPositionHookFunction setSystemMouseCursorPositionHookFunction)
{
	ms_setSystemMouseCursorPositionHookFunction = setSystemMouseCursorPositionHookFunction;
}

// ----------------------------------------------------------------------

void Os::setSetSystemMouseCursorPositionHookFunction2(SetSystemMouseCursorPositionHookFunction setSystemMouseCursorPositionHookFunction)
{
	ms_setSystemMouseCursorPositionHookFunction2 = setSystemMouseCursorPositionHookFunction;
}

// ----------------------------------------------------------------------

void Os::setAcquiredFocusHookFunction(AcquiredFocusHookFunction acquiredFocusHookFunction)
{
	ms_acquiredFocusHookFunction = acquiredFocusHookFunction;
}

// ----------------------------------------------------------------------

void Os::setAcquiredFocusHookFunction2(AcquiredFocusHookFunction acquiredFocusHookFunction)
{
	ms_acquiredFocusHookFunction2 = acquiredFocusHookFunction;
}

// ----------------------------------------------------------------------

void Os::setGetHardwareMouseCursorEnabled(GetHardwareMouseCursorEnabled getHardwareMouseCursorEnabled)
{
	ms_getHardwareMouseCursorEnabled = getHardwareMouseCursorEnabled;
}

// ----------------------------------------------------------------------

void Os::setGetOtherAdapterRectsHookFunction(GetOtherAdapterRectsHookFunction getOtherAdapterRectsHookFunction)
{
	ms_getOtherAdapterRectsHookFunction = getOtherAdapterRectsHookFunction;
}

// ----------------------------------------------------------------------

void Os::setWindowPositionChangedHookFunction(WindowPositionChangedHookFunction windowPositionChangedHookFunction)
{
	ms_windowPositionChangedHookFunction = windowPositionChangedHookFunction;
}

//-----------------------------------------------------------------

void Os::setDisplayModeChangedHookFunction(DisplayModeChangedHookFunction displayModeChangedHookFunction)
{
	ms_displayModeChangedHookFunction = displayModeChangedHookFunction;
}

//-----------------------------------------------------------------

void Os::setInputLanguageChangedHookFunction(InputLanguageChangedHookFunction inputLanguageChangedHookFunction)
{
	ms_inputLanguageChangedHookFunction = inputLanguageChangedHookFunction;
}

//-----------------------------------------------------------------
// rls - In IME mode, the F5 key can cause the IME pad window to open up.
// This causes the Japanese player to essentially lock-up because of 
// context issues.  There is probably a better way to handle this, but be
// warned: the SWG input system may not work properly.
void Os::setIMEHookFunction(IMEHookFunction imeHookFunction)
{
	if (ms_IMEHookFunction)
	{
		UnregisterHotKey(ms_window, ms_hotKeyId);
	}

	ms_IMEHookFunction = imeHookFunction;

	// do not install the hotkey fix if the debugger is present.
	if (ms_IMEHookFunction && !OsNamespace::IsDebuggerPresent()) 
	{
		RegisterHotKey(ms_window, ms_hotKeyId, 0, VK_F5);
	}
}

//-----------------------------------------------------------------

void Os::setQueueKeyDownHookFunction(QueueKeyDownHookFunction queueKeyDownHookFunction)
{
	ms_queueKeyDownHookFunction = queueKeyDownHookFunction;
}

//-----------------------------------------------------------------
/**
* Create the specified directory and all of it's parents.
* @param directory the path to a directory
* @return always true currently
*/

bool Os::createDirectories (const char *directory)
{
		//-- construct list of subdirectories all the way down to root
	std::stack<std::string> directoryStack;

	std::string currentDirectory = directory;

	static const char path_seps [] = { '\\', '/', 0 };

	// build the stack
	while (!currentDirectory.empty())
	{
		// remove trailing backslash
		if (currentDirectory[currentDirectory.size()-1] == '\\' || currentDirectory[currentDirectory.size()-1] == '/')
			IGNORE_RETURN(currentDirectory.erase(currentDirectory.size()-1));

		if (currentDirectory[currentDirectory.size()-1] == ':')
		{
			// we've hit something like c:
			break;
		}

		if (!currentDirectory.empty())
			directoryStack.push(currentDirectory);

		// now strip off current directory
		const size_t previousDirIndex = currentDirectory.find_last_of (path_seps);
		if (static_cast<int>(previousDirIndex) == currentDirectory.npos)
			break;
		else
			IGNORE_RETURN(currentDirectory.erase(previousDirIndex));
	}

	//-- build all directories specified by the initial directory
	while (!directoryStack.empty())
	{
		// get the directory
		currentDirectory = directoryStack.top();
		directoryStack.pop();

		// try to create it (don't pass any security attributes)
		IGNORE_RETURN (CreateDirectory(currentDirectory.c_str(), NULL));
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Write out a file.
 * 
 * The file name and where the file is written is system-dependent.
 * 
 * @param fileName  Name of the file to write
 * @param data  Data buffer to write to the file
 */

bool Os::writeFile(const char *fileName, const void *data, int length)     // Length of the data bufferto write
{
	BOOL   result;
	HANDLE handle;
	DWORD  written;

	// open the file for writing
	handle = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// check if it was opened
	if (handle == INVALID_HANDLE_VALUE)
	{
		WARNING (true, ("Os::writeFile unable to create file [%s] for writing.", fileName));
		return false;
	}

	// attempt to write the data
	result = WriteFile(handle, data, static_cast<DWORD>(length), &written, NULL);

	// make sure the data was written okay
	if (!result || written != static_cast<DWORD>(length))
	{
		WARNING (true, ("Os::writeFile error  writing file [%s].  Wrote %d, attempted to write %d.", fileName, written, length));
		static_cast<void>(CloseHandle(handle));
		return false;
	}

	// close the file
	result = CloseHandle(handle);

	// make sure the close was sucessful
	if (!result)
	{
		WARNING (true, ("Os::writeFile error closing file [%s].", fileName));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------
/**
 * Check to see if any child threads have Fataled.
 * 
 * This routine will 
 */

void Os::checkChildThreads()
{
	FATAL(ms_threadDied, ("child thread died"));
}

// ----------------------------------------------------------------------
/**
 * Change the priority of this process.
 * 
 */

void Os::setProcessPriority(Priority priority)
{
	switch (priority)
	{
		case P_low:
			SetPriorityClass(GetCurrentProcess(),  IDLE_PRIORITY_CLASS);
			break;

		case P_normal:
			SetPriorityClass(GetCurrentProcess(),  NORMAL_PRIORITY_CLASS);
			break;

		case P_high:
			SetPriorityClass(GetCurrentProcess(),  HIGH_PRIORITY_CLASS);
			break;

		default:
			DEBUG_FATAL(true, ("Invalid priority"));
			break;
	}
}

// ----------------------------------------------------------------------
/**
 * Update the Os subsystem.
 * 
 * This routine is supported for all platforms.
 * 
 * For the Win* platforms, this routine will process the windows message pump.
 */

bool Os::update()
{
	MSG  msg;
	int  result;

	FloatingPointUnit::update();

	ms_wasFocusLost = false;
	++ms_numberOfUpdates;

#if PRODUCTION == 0

	if (ms_validateGuardPatterns || ms_validateFreePatterns)
	{
		PROFILER_AUTO_BLOCK_DEFINE ("validate heap");
		MemoryManager::verify(ms_validateGuardPatterns, ms_validateFreePatterns);
	}

#endif

	if (ms_processMessagePump)
	{
		do
		{
			checkChildThreads();

			// while there are messages in the queue
			while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				result = GetMessage(&msg, NULL, 0, 0);

				if (result < 0)
				{
					// error, ignore GetMessage
				}
				else
					// get the message
					if (result > 0)
					{
						static_cast<void>(TranslateMessage(&msg));
						static_cast<void>(DispatchMessage(&msg));
					}
					else
					{
						// WM_QUIT handled here
						ms_gameOver = true;
						return false;
					}
			}

			// may need to reprocess the message queue now
		} while (handleDebugMenu());
	}

	Clock::update();

	return true;
}

// ----------------------------------------------------------------------
/**
 * Formats a message error using GetLastError() and FormatMessge().
 * 
 * The buffer returned from this function is dynamically allocated to prevent
 * issues with this routine being called from multiple threads.  The caller
 * must delete the buffer when it is done.
 * 
 * @return A dynamically allocated buffer containing the error message
 */

char *Os::getLastError()
{
	char buffer[2048];

	const DWORD result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);

	if (result == 0)
		return NULL;

	return DuplicateString(buffer);
}

// ----------------------------------------------------------------------

bool Os::handleDebugMenu()
{
#if PRODUCTION == 0

	// pop up the menu if it is wanted and GDI is actually visible
	if (ms_wantPopupDebugMenu && ms_isGdiVisibleHookFunction && ms_isGdiVisibleHookFunction())
	{
		ms_focused = false;
		// unaquire all the dinput devices
		if (ms_lostFocusHookFunction)
			ms_lostFocusHookFunction();

		BOOL  b;
		POINT p;

		// get the upper left corner of the client space in screen coordinates
		p.x = 0;
		p.y = 0;
		b = ClientToScreen(ms_window, &p);
		DEBUG_FATAL(!b, ("ClientToScreen failed"));

		typedef std::map<char *, HMENU, StringCompare> Map;
		Map map;

		// create the menu
		HMENU menu = CreatePopupMenu();
		const char *lastSection = "";
		HMENU lastSubmenu = NULL;
		int index = 1;
		DebugFlags::FlagVector::const_iterator end = DebugFlags::ms_flagsSortedByName.end();
		for (DebugFlags::FlagVector::const_iterator i = DebugFlags::ms_flagsSortedByName.begin(); i != end; ++i, ++index)
		{
			const DebugFlags::Flag &flag = *i;

			// check if we are starting a new section
			if (strcmp(lastSection, flag.section) != 0)
			{
				lastSection = flag.section;

				char buffer[512];
				strcpy(buffer, lastSection);
				char *start = buffer;
				char *slash = NULL;
				lastSubmenu = menu;
				while ((slash = strchr(start, '/')) != NULL)
				{
					*slash = '\0';

					Map::iterator entry = map.find(buffer);
					if (entry == map.end())
					{
						const int numberOfItems = GetMenuItemCount(lastSubmenu);
						int insertLocation = 0;
						for (insertLocation = 0; insertLocation < numberOfItems && GetSubMenu(lastSubmenu, insertLocation); ++insertLocation)
							;

						HMENU newMenu = CreatePopupMenu();
						static_cast<void>(InsertMenu(lastSubmenu, insertLocation, MF_BYPOSITION | MF_POPUP, reinterpret_cast<UINT>(newMenu), start));
						lastSubmenu = newMenu;
						map.insert(Map::value_type(DuplicateString(buffer), lastSubmenu));
					}
					else
						lastSubmenu = entry->second;

					*slash = '/';
					start = slash + 1;
				}

				const int numberOfItems = GetMenuItemCount(lastSubmenu);
				int insertLocation = 0;
				for (insertLocation = 0; insertLocation < numberOfItems && GetSubMenu(lastSubmenu, insertLocation); ++insertLocation)
					;

				HMENU newMenu = CreatePopupMenu();
				static_cast<void>(InsertMenu(lastSubmenu, insertLocation, MF_BYPOSITION | MF_POPUP, reinterpret_cast<UINT>(newMenu), start));
				lastSubmenu = newMenu;
				map.insert(Map::value_type(DuplicateString(buffer), lastSubmenu));
			}

			// add the current flag to the current menu
			static_cast<void>(AppendMenu(lastSubmenu, MF_ENABLED | MF_STRING | (*flag.variable ? MF_CHECKED : MF_UNCHECKED), index, flag.name));
		}

		{
			ms_debugKeyIndex = index;

			Map::iterator entry = map.find("SharedDebug");
			DEBUG_FATAL(entry == map.end(), ("Could not find SharedDebug section"));

			HMENU newMenu = CreatePopupMenu();
			static_cast<void>(AppendMenu(entry->second, MF_POPUP, reinterpret_cast<UINT>(newMenu), "DebugKey"));
			lastSubmenu = newMenu;

			DebugKey::FlagVector::const_iterator end = DebugKey::ms_flags.end();
			for (DebugKey::FlagVector::const_iterator i = DebugKey::ms_flags.begin(); i != end; ++i, ++index)
			{
				// add the current flag to the current menu
				const DebugKey::Flag &flag = *i;
				static_cast<void>(AppendMenu(lastSubmenu, MF_ENABLED | MF_STRING | (*flag.variable ? MF_CHECKED : MF_UNCHECKED), index, flag.name));
			}
		}

		// free the map memory
		while (!map.empty())
		{
			Map::iterator i = map.begin();
			char *value = i->first;
			map.erase(i);
			delete [] value;
		}

		// pop up the menu at the top corner of the client space
		index  = TrackPopupMenuEx(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON, p.x, p.y, ms_window, NULL);
		if (index)
		{
			if (index < ms_debugKeyIndex)
			{
				bool &value = *DebugFlags::ms_flagsSortedByName[index-1].variable;
				value = !value;
			}
			else
			{
#if PRODUCTION == 0
				DebugKey::setCurrentFlag(DebugKey::ms_flags[index - ms_debugKeyIndex].variable);
#endif
			}
		}

		DestroyMenu(menu);
		
		// don't want the menu anymore
		ms_wantPopupDebugMenu = false;
		ms_wasFocusLost = true;
		ms_focused = true;
		if (ms_acquiredFocusHookFunction2)
			ms_acquiredFocusHookFunction2();

		return true;
	}

#endif

	return false;
}

// ----------------------------------------------------------------------

void Os::enablePopupDebugMenu()
{
#if PRODUCTION == 0
	ms_allowPopupDebugMenu = true;
#endif
}

// ----------------------------------------------------------------------
/**
 * Request that the popup debug menu be displayed.
 * 
 * The popup debug menu will only be displayed if the proper config file
 * switch has been set.
 */

void Os::requestPopupDebugMenu()
{
#if PRODUCTION == 0
	ms_wantPopupDebugMenu = ms_allowPopupDebugMenu;
#endif
}

bool Os::isNumPadValue(unsigned char asciiChar)
{
	return (asciiChar >= '0' && asciiChar <= '9') ||
		   (asciiChar == '/') ||
		   (asciiChar == '*') ||
		   (asciiChar == '-') ||
		   (asciiChar == '+') ||
		   (asciiChar == '.');
}


// ----------------------------------------------------------------------
/**
 * Check to see if this is a NumPad keypress to be consumed.  Enter is the only one not consumed.
 */
bool Os::isNumPadChar(unsigned char asciiChar)
{
	BYTE keyboardstate[256];
	GetKeyboardState( keyboardstate );

	if (isNumPadValue(asciiChar))
	{
		return ( (keyboardstate[VK_NUMPAD1] > 1) ||
				 (keyboardstate[VK_NUMPAD2] > 1) ||
				 (keyboardstate[VK_NUMPAD3] > 1) ||
				 (keyboardstate[VK_NUMPAD4] > 1) ||
				 (keyboardstate[VK_NUMPAD5] > 1) ||
				 (keyboardstate[VK_NUMPAD6] > 1) ||
				 (keyboardstate[VK_NUMPAD7] > 1) ||
				 (keyboardstate[VK_NUMPAD8] > 1) ||
				 (keyboardstate[VK_NUMPAD9] > 1) ||
				 (keyboardstate[VK_NUMPAD0] > 1) ||
				 (keyboardstate[VK_DIVIDE] > 1) ||
				 (keyboardstate[VK_ADD] > 1) ||
				 (keyboardstate[VK_SUBTRACT] > 1) ||
				 (keyboardstate[VK_MULTIPLY] > 1) ||
				 (keyboardstate[VK_DECIMAL] > 1));
	}
	return false;
}

// ----------------------------------------------------------------------
/**
 * Handle window messages.
 * 
 * This routine will process window messages that are passed into the application from
 * Windows, likely though the Os::update() routine, but may be from other locations as well.
 * 
 * @param hwnd  Handle of window
 * @param uMsg  Message identifier
 * @param wParam  First message parameter
 * @param lParam  Second message parameter
 * @see Os::update()
 */

LRESULT CALLBACK Os::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if 0
	DEBUG_REPORT_LOG_PRINT(true, ("%08d %08x %08x %08x\n", ms_numberOfUpdates, uMsg, wParam, lParam));
#endif

	if (ms_IMEHookFunction)
	{
		// Let the IME Manager see and possibly consume message
		if (ms_IMEHookFunction(hwnd, uMsg, wParam, lParam) == 0)
		{
			return 0;
		}
	}

	switch (uMsg)
	{
		case WM_ERASEBKGND:
			// won't let windows erase the background
			return 0;

		case WM_IME_CHAR:
			if (ms_queueCharacterHookFunction)
			{			
				const char ansiChars [2] =
				{
					static_cast<char>(HIBYTE( wParam )), //lint !e1924 // c-style case msvc bug
					static_cast<char>(LOBYTE( wParam ))  //lint !e1924 // c-style case msvc bug
				};	

				wchar_t u;
				if (MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, reinterpret_cast<const char *>(ansiChars), 2, &u, 1))
					ms_queueCharacterHookFunction(0, static_cast<int>(u)); 
			}
			return 0;

		case WM_HOTKEY:
			{
				if (ms_queueKeyDownHookFunction) 
				{
					ms_queueKeyDownHookFunction(0, MapVirtualKey(HIWORD(lParam), 0));
				}
				return 0;
			}
		case WM_CHAR:
			// handle typed string characters
			if (ms_queueCharacterHookFunction)
			{
				//-- the extended bit is bit 24
				const int extended = (lParam & (1 << 24)) != 0;
	
				if (!extended)
				{
					int keyCode = (lParam << 8) >> 24;  // key code is in bits 16-23
					// cp* == ASCII until 0x80, for which you then need to call the following to get the Unicode value
					// from the cp* value.  WM_CHAR always returns the values from the windows codepage (cp) used.  For US/Europe
					// it is cp1252 and for Japan it is cp932.
					if (wParam >= 0x80)  
					{
						char cpChar[2];
						wchar_t unicodeChar[2];

						cpChar[0] = static_cast<char>(wParam);
						cpChar[1] = '\0';

						int result = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, cpChar, sizeof(cpChar), unicodeChar, sizeof(unicodeChar));


						if (result > 0)
						{
							ms_queueCharacterHookFunction(keyCode, static_cast<int>(unicodeChar[0]));
						}
					}
					else
					{
						ms_queueCharacterHookFunction(keyCode, static_cast<int>(wParam));
					}					
				}
			}
			return 0;

		case WM_INPUTLANGCHANGE:
			{
				if (ms_inputLanguageChangedHookFunction)
				{
					ms_inputLanguageChangedHookFunction();
				}
			}
			return 0;

		case WM_DESTROY:
			// if the main window gets destroyed, it's time to quit
			PostQuitMessage(0);
			return 0;

		case WM_SYSCOMMAND:
			switch (wParam & 0xFFF0)
			{
				// hack hack hack hack hack
				// don't let alt-f4 close the window, but allow clicking on the close X button to close the window
				case SC_CLOSE:
					if (GetKeyState(VK_F4) && (GetKeyState(VK_MENU) || GetKeyState(VK_RMENU)))
						return 0;
					break;

				// don't let the monitor get turned off
				case SC_MONITORPOWER:
					return 0;

				// don't allow the screen saver to come on
				case SC_SCREENSAVE:
					return 0;

				// don't allow alt-space to open up the window menu
				case SC_KEYMENU:
					return 0;

				case SC_MOVE:
					ClipCursor(NULL);
					ms_focused = false;
					if (ms_lostFocusHookFunction)
						ms_lostFocusHookFunction();
					ms_wasFocusLost = true;
					break;
				default:
					break;
			}
			break;

		case WM_ENTERSIZEMOVE:
			if (ms_getOtherAdapterRectsHookFunction)
			{
				ms_otherAdapterRects.clear();
				(*ms_getOtherAdapterRectsHookFunction)(ms_otherAdapterRects);
			}
			break;

		case WM_EXITSIZEMOVE:
			ms_focused = true;
			if (ms_acquiredFocusHookFunction)
				ms_acquiredFocusHookFunction();
			if (ms_acquiredFocusHookFunction2)
				ms_acquiredFocusHookFunction2();
			ms_wasFocusLost = true;
			if (ms_windowPositionChangedHookFunction)
				(*ms_windowPositionChangedHookFunction)();
#if PRODUCTION == 0
			DebugMonitor::setBehindWindow(ms_window);
#endif
			break;

		case WM_MOUSEACTIVATE:
			if (hwnd == ms_window)
			{
				if (LOWORD(lParam) == HTCAPTION)
					ms_clickToMove = true;
				else if (LOWORD(lParam) == HTCLIENT)
				{
					ms_focused = true;
					if (ms_acquiredFocusHookFunction)
						ms_acquiredFocusHookFunction();
					if (ms_acquiredFocusHookFunction2)
						ms_acquiredFocusHookFunction2();
				}
			}

			break;

		case WM_MOUSEMOVE:
			if (ms_focused && hwnd == ms_window)
			{
				ms_mouseMoveInClient = true;
				updateMousePosition(LOWORD(lParam), HIWORD(lParam));
			}
			break;

		case WM_NCMOUSEMOVE:
			ms_mouseMoveInClient = false;
			break;

		case WM_SETCURSOR:
			if (!ms_mouseMoveInClient || hwnd != ms_window)
				SetCursor(ms_cursorArrow);
			else
				if (!ms_getHardwareMouseCursorEnabled || !ms_getHardwareMouseCursorEnabled())
					SetCursor(NULL);
			break;

		case WM_NCACTIVATE:
#if PRODUCTION == 0
			// hack to handle coming back from the debugger cleanly
			if (wParam)
			{
				//allow game-specific systems a chance to respond to focus changes
				DebugMonitor::setBehindWindow(ms_window);
				if (ms_lostFocusHookFunction)
					ms_lostFocusHookFunction();
				if (ms_acquiredFocusHookFunction)
					ms_acquiredFocusHookFunction();
				if (ms_acquiredFocusHookFunction2)
					ms_acquiredFocusHookFunction2();
				ms_wasFocusLost = true;
				ms_focused = true;
			}
#endif
			break;

		case WM_ACTIVATE:
			if (hwnd == ms_window)
			{
				if (wParam != WA_INACTIVE)
				{
					if (ms_clickToMove)
						ms_clickToMove = false;
					ms_mouseMoveInClient = true;
					ms_focused = true;
					ms_wasFocusLost = true;

					if (ms_acquiredFocusHookFunction)
						ms_acquiredFocusHookFunction();
					if (ms_acquiredFocusHookFunction2)
						ms_acquiredFocusHookFunction2();
				}
				else
				{
					ClipCursor(NULL);
					ms_focused = false;
					if (ms_lostFocusHookFunction)
						ms_lostFocusHookFunction();
				}
			}
			break;
			
		case WM_ACTIVATEAPP:
			if (wParam == FALSE)
			{
				ClipCursor(NULL);
				ms_focused = false;
				if (ms_lostFocusHookFunction)
					ms_lostFocusHookFunction();
			}
			break;

		case WM_DISPLAYCHANGE:
			if (ms_displayModeChangedHookFunction)
				ms_displayModeChangedHookFunction();
			break;

		default:
			break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------------------------

void OsNamespace::updateMousePosition(int x, int y)
{
	if (ms_setSystemMouseCursorPositionHookFunction)
		ms_setSystemMouseCursorPositionHookFunction(x, y);

	if (ms_setSystemMouseCursorPositionHookFunction2)
		ms_setSystemMouseCursorPositionHookFunction2(x, y);
}

// ----------------------------------------------------------------------
/**
 * Get an identifier indicating which thread called this function.
 */

Os::ThreadId Os::getThreadId()
{
	return GetCurrentThreadId();
}

// ----------------------------------------------------------------------
/**
 * Get the actual system time, in seconds since the epoch.
 *
 * Do not use this for most game systems, since it does not take into account
 * clock sku, game loop times, etc.
 */
time_t Os::getRealSystemTime()
{
	return time(0);
}

// ----------------------------------------------------------------------
/**
 * Convert a time in seconds since the epoch to GMT.
 *
 */

void Os::convertTimeToGMT(const time_t &convertTime, tm &zulu)
{
	zulu=*gmtime(&convertTime); // gmtime uses a single static tm structure.  Yuck!
}

// ----------------------------------------------------------------------
/**
 * Convert a tm structure to the time in seconds since the epoch.
 *
 */

time_t Os::convertGMTToTime(const tm &zulu)
{
	return mktime(const_cast<tm*>(&zulu));
}

// ----------------------------------------------------------------------
/**
 * Cause the current thread to suspend for a period of time.  Zero delay indicates
 * to yield the current time slice.
 */

void Os::sleep(int ms)
{
	::Sleep(static_cast<DWORD>(ms));
}

// ----------------------------------------------------------------------
/**
 * Assign the given thread a reasonable name (only works for MSDev 6.0 debugger)
 * Max 9 characters
 *
 */

void Os::setThreadName(ThreadId threadID, const char* threadName)
{
	//used to give threads reasonable names in the MSDev debugger,
	//see http://www.vcdj.com/upload/free/features/vcdj/2001/03mar01/et0103/et0103.asp for more info
	struct ThreadNameInfo
	{
		DWORD dwType;
		LPCSTR szName;							
		DWORD dwThreadID;
		DWORD dwFlags;
	};

	ThreadNameInfo info;
	info.dwType = 0x1000;       //must be this value
	info.szName = threadName;
	info.dwThreadID = threadID;
	info.dwFlags = 0;           //unused, reserved for future use

	__try
	{
		// use the magic exception number MS picked for this purpose
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), reinterpret_cast<DWORD *>(&info));
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

// ----------------------------------------------------------------------
/**
 * Given a base directory and a full file pathname, find the relative path
 * needed to get from the source directory to the target.
 *
 * This command is case sensitive.  It does not care whether baseDirectory contains
 * a trailing backslash or not.
 *
 * @param baseDirectory     the base directory from which a relative path will be constructed.
 * @param targetPathname    the full pathname for the file for which we want to generate a relative path
 * @param relativePath      the string into which the constructed relative path will be returned
 */

void Os::buildRelativePath(const char *baseDirectory, const char *targetPathname, std::string &relativePath)
{
	NOT_NULL(baseDirectory);
	DEBUG_FATAL(!targetPathname || !*targetPathname, ("bad targetPathname, must be non-zero length"));

	const char        directorySeparator    = '\\';
	const char *const backupDirectoryString = "..\\";

	std::string  workingBaseDirectory(baseDirectory);

	//-- ensure base directory ends in directory separator
	if (workingBaseDirectory[workingBaseDirectory.length()-1] != directorySeparator)
		workingBaseDirectory += directorySeparator;

	//-- grab the target directory
	std::string  workingTargetDirectory;

	const char *endOfDirectory = strrchr(targetPathname, static_cast<int>(directorySeparator));
	if (endOfDirectory)
		IGNORE_RETURN(workingTargetDirectory.append(targetPathname, static_cast<size_t>(endOfDirectory - targetPathname + 1)));

	//-- find count of character of match between directory strings
	const size_t workingBaseDirectoryLength   = workingBaseDirectory.length();
	const size_t workingTargetDirectoryLength = workingTargetDirectory.length();

	size_t searchIndex = 0;
	while ((searchIndex < workingBaseDirectoryLength) && (searchIndex < workingTargetDirectoryLength) && (workingBaseDirectory[searchIndex] == workingTargetDirectory[searchIndex]))
		++searchIndex;

	size_t matchCount = searchIndex;

	//-- if we match into the middle of a directory, back up until the previous directory
	if ((matchCount > 0) && (workingBaseDirectory[matchCount - 1] != directorySeparator))
	{
		do
		{
			--matchCount;
		} while ((matchCount > 0) && (workingBaseDirectory[matchCount - 1] != directorySeparator));
	}

	//-- if we have no match between directories, the best path is the full path.
	if (matchCount < 1)
	{
		relativePath = targetPathname;
		return;
	}

	//-- for each directory in base directory not matched, insert a "backup directory" string
	relativePath = "";
	{
		for (size_t i = matchCount; i < workingBaseDirectoryLength; ++i)
			if (workingBaseDirectory[i] == directorySeparator)
				IGNORE_RETURN(relativePath.append(backupDirectoryString));
	}

	//-- for each directory in the target directory not matched, append to result.
	//   this works out to copying from the match point forward in the target path.
	IGNORE_RETURN(relativePath.append(targetPathname + matchCount));
}

// ----------------------------------------------------------------------
/**
 * Convert a relative path to an absolute path.
 */
bool Os::getAbsolutePath(const char *relativePath, char *absolutePath, int absolutePathBufferSize)
{
	if (_fullpath(absolutePath, relativePath, static_cast<size_t>(absolutePathBufferSize)) == NULL)
		return false;

	// convert the slashes to be forwards
	for (char *convert = absolutePath; *convert; ++convert)
		if (*convert == '\\')
			*convert = '/';

	return true;
}

// ----------------------------------------------------------------------
/**
 * Copy text to the system clipboard.
 *
 * This routine can be used if the application wants to make some data easily available for pasting (like crash call stacks).
 */

bool Os::copyTextToClipboard(const char *text)
{
	if (!OpenClipboard(ms_window))
		return false;

	if (!EmptyClipboard())
		return false;

	// need to convert to cr/lf sequences.  yuck.
	int length = 1;
	for (const char *t2 = text; *t2; ++t2, ++length)
	{
		if (*t2 == '\n')
			++length;
	}

	HANDLE memoryHandle = GlobalAlloc(GMEM_MOVEABLE, length);
	if (memoryHandle == NULL) 
	{ 
		CloseClipboard(); 
		return FALSE; 
	} 

	// lock the handle and copy the text to the buffer. 
	char *destination = reinterpret_cast<char *>(GlobalLock(memoryHandle));
	while (*text)
	{
		if (*text == '\n')
			*(destination++) = '\r';
		*destination++ = *text++;
	}

	if (GlobalUnlock(memoryHandle) != 0 || GetLastError() != NO_ERROR)
	{
		IGNORE_RETURN(GlobalFree(memoryHandle));
		return false;
  }

	if (!SetClipboardData(CF_TEXT, memoryHandle))
		return false;

	if (!CloseClipboard())
		return false;

	return true;
}

// ----------------------------------------------------------------------

bool Os::getUserName(char *buffer, int &bufferSize)
{
	NOT_NULL(buffer);
	DWORD windowsBufferSize = static_cast<DWORD>(bufferSize);
	buffer[0] = '\0';
	bool result = GetUserName(buffer, &windowsBufferSize) == TRUE;
	bufferSize = static_cast<int>(bufferSize);
	return result;
}

//-----------------------------------------------------------------------

Os::OsPID_t Os::getProcessId()
{
	return static_cast<OsPID_t>(GetCurrentProcessId());
}

//----------------------------------------------------------------------

bool Os::isFocused()
{
	return ms_focused;
}

//----------------------------------------------------------------------

bool Os::launchBrowser(std::string const & website)
{
	std::string URL("http://");
	if (strncmp(URL.c_str(), website.c_str(),7)!=0)
		URL+=website;
	else
		URL=website;
	int result = reinterpret_cast<int>(ShellExecute(NULL, "open", URL.c_str(), NULL, NULL, SW_SHOWNORMAL));
	return (result > 32);
}


// ======================================================================
