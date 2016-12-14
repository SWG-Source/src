// ======================================================================
//
// Os.h
//
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_Os_H
#define INCLUDED_Os_H

// ======================================================================

#include <ctime>

// ======================================================================

class Os
{
public:

	typedef bool (*IsGdiVisibleHookFunction)();
	typedef void (*LostFocusHookFunction)();
	typedef void (*AcquiredFocusHookFunction)();
	typedef void (*QueueCharacterHookFunction)(int keyboard, int character);
	typedef void (*SetSystemMouseCursorPositionHookFunction)(int x, int y);
	typedef bool (*GetHardwareMouseCursorEnabled)();
	typedef void (*GetOtherAdapterRectsHookFunction)(std::vector<RECT> &);
	typedef void (*WindowPositionChangedHookFunction)();
	typedef void (*DisplayModeChangedHookFunction)();
	typedef void (*InputLanguageChangedHookFunction)();
	typedef int (*IMEHookFunction)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	typedef void (*QueueKeyDownHookFunction)(int keyboard, int character);

	typedef uint   ThreadId;
	typedef DWORD  OsPID_t;

	enum Priority
	{
		P_high,
		P_normal,
		P_low
	};

	enum
	{
		MAX_PATH_LENGTH = 512
	};

public:

	static void                install(HINSTANCE newInstance, const char *windowName, HICON normalIcon, HICON smallIcon);
	static void                install(HWND newWindow, bool processMessagePump);
	static void                install();

	static bool                isGameOver();
	static DLLEXPORT bool      isMainThread();
	static bool                wasFocusLost();
	static void                checkChildThreads();

	static void                setProcessPriority(Priority priority);

	static bool                update();

	static void                returnFromAbort();
	static void                abort();

	static void                enablePopupDebugMenu();
	static void                requestPopupDebugMenu();

	static bool                createDirectories (const char *dirname);

	static bool                writeFile(const char *fileName, const void *data, int length);

	static HWND                getWindow();
	static bool                engineOwnsWindow();

	static int                 getNumberOfUpdates();

	static char               *getLastError();

	static const char         *getProgramName();
	static const char         *getShortProgramName();
	static const char         *getProgramStartupDirectory();

	static void                setIsGdiVisibleHookFunction(IsGdiVisibleHookFunction newGlIsGdiVisible);
	static void                setLostFocusHookFunction(LostFocusHookFunction lostFocusHookFunction);
	static void                setQueueCharacterHookFunction(QueueCharacterHookFunction queueCharacterHookFunction);
	static void                setSetSystemMouseCursorPositionHookFunction(SetSystemMouseCursorPositionHookFunction setSystemMouseCursorPositionHookFunction);
	static void                setSetSystemMouseCursorPositionHookFunction2(SetSystemMouseCursorPositionHookFunction setSystemMouseCursorPositionHookFunction);
	static void                setAcquiredFocusHookFunction(AcquiredFocusHookFunction acquiredFocusHookFunction);
	static void                setAcquiredFocusHookFunction2(AcquiredFocusHookFunction acquiredFocusHookFunction);
	static void                setGetHardwareMouseCursorEnabled(GetHardwareMouseCursorEnabled getHardwareMouseCursorEnabled);
	static void                setGetOtherAdapterRectsHookFunction(GetOtherAdapterRectsHookFunction getOtherAdapterRectsHookFunction);
	static void                setWindowPositionChangedHookFunction(WindowPositionChangedHookFunction windowPositionChangedHookFunction);
	static void                setDisplayModeChangedHookFunction(DisplayModeChangedHookFunction displayModeChangedHookFunction);
	static void                setInputLanguageChangedHookFunction(InputLanguageChangedHookFunction inputLanguageChangedHookFunction);
	static void                setIMEHookFunction(IMEHookFunction imeHookFunction);
	static void                setQueueKeyDownHookFunction(QueueKeyDownHookFunction queueKeyDownHookFunction);

	static DLLEXPORT ThreadId  getThreadId();
	static void                setThreadName(ThreadId threadID, const char* name);

	static void                sleep(int ms);

	static time_t              getRealSystemTime();
	static void                convertTimeToGMT(const time_t &time, tm &zulu);
	static time_t              convertGMTToTime(const tm &zulu);

	static bool                isMultiprocessor();
	static int                 getProcessorCount();

	static void                buildRelativePath(const char *baseDirectory, const char *targetPathname, std::string &relativePath);
	static bool                getAbsolutePath(const char *relativePath, char *absolutePath, int absolutePathBufferSize);

	static bool                copyTextToClipboard(const char *text);

	static bool                getUserName(char *buffer, int &bufferSize);

	static OsPID_t             getProcessId();

	static bool                isFocused();

	static bool                launchBrowser(std::string const & website);
	static bool				   isNumPadValue(unsigned char asciiChar);
	static bool				   isNumPadChar(unsigned char asciiChar);

private:

	Os();
	Os(const Os &);
	Os &operator =(const Os &);

private:

	static void remove();
	static void installCommon();

	static LRESULT CALLBACK Os::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

private:

	static bool            handleDebugMenu();
};

// ======================================================================

#endif
