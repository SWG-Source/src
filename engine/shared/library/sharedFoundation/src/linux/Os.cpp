// ======================================================================
//
// Os.cpp
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Os.h"

#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FloatingPointUnit.h"
#include "sharedIoWin/IoWinManager.h"

#include <stack>
#include <string>

// ======================================================================

bool            Os::installed;
bool            Os::runInBackground;
int             Os::numberOfUpdates;
int             Os::menuValue;
bool            Os::paused;
bool            Os::wasPaused;
bool            Os::gameOver;
bool            Os::shouldReturnFromAbort;
char            Os::programName[PROGRAM_NAME_SIZE];
char           *Os::shortProgramName;
pthread_t       Os::mainThreadId;
bool            Os::threadDied;

bool            Os::isMp;
int             Os::processorCount;

// ----------------------------------------------------------------------

namespace OsNamespace
{
	class UncatchableException
	{
	};
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

void Os::install(void)
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

void Os::remove(void)
{
	DEBUG_FATAL(!installed, ("not installed"));
	installed = false;
}

// ----------------------------------------------------------------------

void Os::installCommon(void)
{
	DEBUG_FATAL(installed, ("already installed"));

	ExitChain::add(Os::remove, "Os::remove", 0, true);

	numberOfUpdates = 0;
	mainThreadId = pthread_self();

	// get the name of the executable
//Can't find UNIX call for this: DWORD result = GetModuleFileName(nullptr, programName, sizeof(programName));
        strcpy(programName, "TempName");
        DWORD result = 1;


	FATAL(result == 0, ("GetModuleFileName failed"));

	// get the file name without the path
	shortProgramName = strrchr(programName, '\\');
	if (shortProgramName)
		++shortProgramName;
	else
		shortProgramName = programName;

	// determine the number of processors by parsing /proc/cpuinfo
	processorCount = 1;
	FILE * f = fopen("/proc/cpuinfo", "r");
	if (f)
	{
		char buffer[512];
		while (!feof(f))
		{
			if (fgets(buffer, 512, f) != nullptr) {
				if (strncmp(buffer, "processor\t: ", 12)==0)
				{
					processorCount = atoi(buffer+12)+1;
				}
			}
		}
		fclose(f);
	}
	isMp = processorCount > 1;

	// switch into single-precision floating point mode
	FloatingPointUnit::install();

	installed = true;
}

// ----------------------------------------------------------------------

bool Os::isMainThread(void)
{
	// if the Os class hasn't been installed, then assume we are the main thread.
	// otherwise, check to see if our thread id is the main thread id
	return !installed || (pthread_self() == mainThreadId);
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
#include <signal.h>
void Os::abort(void)
{
	if (!isMainThread())
	{
		threadDied = true;
		pthread_exit(nullptr);
	}

	if (!shouldReturnFromAbort)
	{
		// let the C runtime deal with the abnormal termination
		int * dummy = nullptr;
		int forceCrash = *dummy;
		UNREF(forceCrash);
		for (;;)
		{
			// One of these should work:
			pthread_kill(pthread_self(), SIGSEGV);
			::kill(0,SIGSEGV);
			::abort();
			OsNamespace::UncatchableException ex;
			throw ex;
			sleep(10);
		}
	}
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
		size_t previousDirIndex = currentDirectory.find_last_of (path_seps);
		if (previousDirIndex == currentDirectory.npos)
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
		IGNORE_RETURN (mkdir (currentDirectory.c_str(), 0));
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
	handle = CreateFile(fileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	// check if it was opened
	if (handle == INVALID_HANDLE_VALUE)
		return false;

	// attempt to write the data
	result = WriteFile(handle, data, static_cast<DWORD>(length), &written, nullptr);

	// make sure the data was written okay
	if (!result || written != static_cast<DWORD>(length))
	{
		static_cast<void>(CloseHandle(handle));
		return false;
	}

	// close the file
	result = CloseHandle(handle);

	// make sure the close was sucessful
	if (!result)
		return false;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Update the Os subsystem.
 *
 * This routine is supported for all platforms.
 *
 * For the Win* platforms, this routine will process the windows message pump.
 */

bool Os::update(void)
{
	static int ppid = getppid();

    FloatingPointUnit::update();

    ++numberOfUpdates;

    Clock::update();

    wasPaused = false;

		// if our parent's pid has changed, we tell our caller it should exit
		if (getppid() != ppid)
		{
			WARNING(true, ("Parent process exited!"));
			// reset the parent process id, so we don't keep spamming warnings for processes that don't exit because of this
			ppid = getppid();
			return false;
		}

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

char *Os::getLastError(void)
{
	return DuplicateString(strerror(errno));
}

// ----------------------------------------------------------------------

bool Os::getAbsolutePath(const char *relativePath, char *absolutePath, int absolutePathBufferSize)
{
	// realpath sucks and could cause a buffer overrun.  however, it's better than writing it ourselves for now.
	char *result = realpath(relativePath, absolutePath);
	if (!result){
		free(result);
		return false;
	}

	FATAL(istrlen(absolutePath)+1 > absolutePathBufferSize, ("buffer overrun"));
	return true;
}

// ----------------------------------------------------------------------
/**
 * Get the actual system time, in seconds since the epoch.
 *
 * Do not use this for most game systems, since it does not take into account
 * clock sku, game loop times, etc.
 */
time_t Os::getRealSystemTime(void)
{
	return time(0);
}

// ----------------------------------------------------------------------
/**
 * Convert a time in seconds since the epoch to GMT.
 *
 */

void Os::convertTimeToGMT(const time_t &time, tm &zulu)
{
	zulu=*gmtime(&time); // gmtime uses a single static tm structure.  Yuck!
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
 * Get a unique identifier for this thread.
 *
 * @return A unique identifier for this thread.
 */

Os::ThreadId Os::getThreadId()
{
	return pthread_self();
}

// ----------------------------------------------------------------------
/**
 * Assign the given thread a reasonable name (only works for MSDev 6.0 debugger)
 * Since this is Linux-specific, make sure this fuction does nothing
 *
 */

void Os::setThreadName(DWORD threadID, const char* threadName)
{
}

// ----------------------------------------------------------------------
/**
 * Cause the current thread to sleep for a period of time.  If the period is
 * zero, the current thread yields it's timeslice.
 */

void Os::sleep(int ms)
{
	if (ms == 0)
	{
		sched_yield();
	}
	else
	{
		usleep(ms * 1000);
	}
}

// ----------------------------------------------------------------------
/**
 * Copy text to the system clipboard.
 *
 * This routine can be used if the application wants to make some data easily available for pasting (like crash call stacks).
 */

bool Os::copyTextToClipboard(const char *text)
{
	UNREF(text);
	return false;
}

// ----------------------------------------------------------------------

Os::OsPID_t Os::getProcessId()
{
	return getpid();
}

// ----------------------------------------------------------------------

void Os::setProgramName(const char * name)
{
	strncpy(programName, name, Os::PROGRAM_NAME_SIZE);
}

// ----------------------------------------------------------------------

bool Os::isFocused()
{
	return true;
}

// ----------------------------------------------------------------------
