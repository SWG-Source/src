// ======================================================================
//
// Os.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef OS_H
#define OS_H

#include <pthread.h>

// ======================================================================

struct DebugMenuEntry;

// ======================================================================

class Os
{
public:

	typedef pthread_t ThreadId;
	typedef pid_t OsPID_t;

	enum
	{
#ifdef PATH_MAX
		MAX_PATH_LENGTH = PATH_MAX+1
#else
		MAX_PATH_LENGTH = 512
#endif
	};

	typedef void (*QueueCharacterHookFunction)(int keyboard, int character);
	typedef void (*SetSystemMouseCursorPositionHookFunction)(int x, int y);
	typedef void (*QueueKeyDownHookFunction)(int keyboard, int character);


private:

	Os(void);
	Os(const Os &);
	Os &operator =(const Os &);
	
	enum
	{
		PROGRAM_NAME_SIZE = 512
	};

private:

	static void remove(void);

	static void installCommon(void);

private:

	static bool            installed;
	static bool            runInBackground;
	static int             numberOfUpdates;
	static int             menuValue;
	static DebugMenuEntry *debugMenuGame;
	static bool            paused;
	static bool            wasPaused;
	static bool            gameOver;
	static bool            shouldReturnFromAbort;
	static bool            wantPopupDebugMenu;
	static char            programName[PROGRAM_NAME_SIZE];
	static char           *shortProgramName;
	static pthread_t       mainThreadId;
	static bool            threadDied;
	static bool            isMp;
	static int             processorCount;


public:

	static void install(void);

	static bool isGameOver(void);
	static bool isMainThread(void);
	static bool wasApplicationPaused(void);

	static bool update(void);

	static void returnFromAbort(void);
	static void abort(void);

	static void requestPopupDebugMenu();

	static bool createDirectories (const char *dirname);

	static bool writeFile(const char *fileName, const void *data, int length);

	static int  getNumberOfUpdates(void);

	static char *getLastError(void);

	static void     setThreadName(DWORD threadID, const char* name);
	static ThreadId getThreadId();

	static const char *getProgramName(void);
	static const char *getShortProgramName(void);
	static void        setProgramName(const char * name);
	
	static bool getAbsolutePath(const char *relativePath, char *absolutePath, int absolutePathBufferSize);

	static void			sleep(int ms);

	static time_t getRealSystemTime(void);
	static void convertTimeToGMT(const time_t &time, tm &zulu);
	static time_t convertGMTToTime(const tm &zulu);

	static bool isMultiprocessor();
	static int getProcessorCount();

	static bool copyTextToClipboard(const char *text);

	static bool wasFocusLost();
	static void setQueueCharacterHookFunction(QueueCharacterHookFunction queueCharacterHookFunction);
	static void setSetSystemMouseCursorPositionHookFunction(SetSystemMouseCursorPositionHookFunction setSystemMouseCursorPositionHookFunction);
	static OsPID_t  getProcessId();
	static bool isFocused();
	
	static void setQueueKeyDownHookFunction(QueueKeyDownHookFunction queueKeyDownHookFunction);
};

// ----------------------------------------------------------------------
/**
 * Return the full name of the running executable.
 *
 * The program name will include the path as well.
 *
 * @return The full name of the running executable
 * @see Os::getShortProgramName()
 */

inline const char *Os::getProgramName(void)
{
	return programName;
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

inline const char *Os::getShortProgramName(void)
{
	return shortProgramName;
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

inline void Os::returnFromAbort(void)
{
	shouldReturnFromAbort = true;
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

inline bool Os::isGameOver(void)
{
	return gameOver;
}

// ----------------------------------------------------------------------
/**
 * Return the number of updates that the have occurred.
 *
 * @return This value is updated during the Os::update() routine.
 */

inline int Os::getNumberOfUpdates(void)
{
	return numberOfUpdates;
}

// ----------------------------------------------------------------------
/**
 * Indicate whether or not the application was paused.
 *
 * @return True if the application was in the background (paused), otherwise false
 */

inline bool Os::wasApplicationPaused(void)
{
	return wasPaused;
}

// ----------------------------------------------------------------------
/**
 * Return a flag indicating whether we are running a multiprocessor machine or not.
 *
 * @return True if the machine has more than one processor, false if not.
 */

inline bool Os::isMultiprocessor(void)
{
	return isMp;
}

// ----------------------------------------------------------------------
/**
 * Return the number of processors.
 *
 * @return The number of processors in the machine.
 */

inline int Os::getProcessorCount(void)
{
	return processorCount;
}

// ----------------------------------------------------------------------

inline bool Os::wasFocusLost()
{
	return false;
}

// ----------------------------------------------------------------------

inline void Os::setQueueCharacterHookFunction(QueueCharacterHookFunction queueCharacterHookFunction)
{
}

// ----------------------------------------------------------------------

inline void Os::setSetSystemMouseCursorPositionHookFunction(SetSystemMouseCursorPositionHookFunction setSystemMouseCursorPositionHookFunction)
{
}

// ----------------------------------------------------------------------

inline void Os::setQueueKeyDownHookFunction(QueueKeyDownHookFunction)
{
}


// ----------------------------------------------------------------------

#endif
