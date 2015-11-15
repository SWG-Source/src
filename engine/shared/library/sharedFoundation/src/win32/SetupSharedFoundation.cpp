// ======================================================================
//
// SetupSharedFoundation.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/SetupSharedFoundation.h"

#include "sharedDebug/DebugMonitor.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/Profiler.h"

#include "sharedFoundation/ApplicationVersion.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/CommandLine.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/Production.h"
#include "sharedFoundation/RegistryKey.h"
#include "sharedFoundation/StaticCallbackEntry.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Watcher.h"
#include "sharedLog/TailFileLogObserver.h"

#include <eh.h>
#include <cstdio>

// ======================================================================

namespace FatalNamespace
{
	extern char ms_buffer[32 * 1024];
}

namespace SetupSharedFoundationNamespace
{
	LONG __stdcall MyUnhandledExceptionFilter(LPEXCEPTION_POINTERS exceptionPointers);

	bool  ms_writeMiniDumps;
}

using namespace SetupSharedFoundationNamespace;

// ======================================================================

LONG __stdcall SetupSharedFoundationNamespace::MyUnhandledExceptionFilter(LPEXCEPTION_POINTERS exceptionPointers)
{
	// make the routine somewhat safe from re-entrance
	static bool entered = false;
	if (entered)
		return EXCEPTION_CONTINUE_SEARCH;
	entered = true;

	// log some important information
	static char buffer[128];
	sprintf(buffer, "Exception %08x(%d)=code %08x=addr\n", exceptionPointers->ExceptionRecord->ExceptionCode, exceptionPointers->ExceptionRecord->ExceptionCode, exceptionPointers->ExceptionRecord->ExceptionAddress);
	OutputDebugString(buffer);

	// write the minidump if we're in here for the first time
	static bool ms_alreadyWroteMiniDump = false;
	if (ms_writeMiniDumps && !ms_alreadyWroteMiniDump)
	{
		ms_alreadyWroteMiniDump = true;

		uint64 timestamp;
		time_t now;
		tm t;

		IGNORE_RETURN(time(&now));
		IGNORE_RETURN(gmtime_r(&now, &t));
		timestamp = t.tm_year+1900; //lint !e732 !e737 !e776
		timestamp *= 100;
		timestamp += t.tm_mon+1; //lint !e737 !e776
		timestamp *= 100;
		timestamp += static_cast<unsigned int>(t.tm_mday);
		timestamp *= 100;
		timestamp += static_cast<unsigned int>(t.tm_hour);
		timestamp *= 100;
		timestamp += static_cast<unsigned int>(t.tm_min);
		timestamp *= 100;
		timestamp += static_cast<unsigned int>(t.tm_sec);

		static char fileName[512];

		sprintf(fileName, "%s-%s-%I64d.txt", Os::getShortProgramName(), ApplicationVersion::getInternalVersion(), timestamp);
		HANDLE const file = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (file != INVALID_HANDLE_VALUE)
		{
			char text1[] = "automated crash dump from ";
			DWORD bytesWritten;
			WriteFile(file, text1, strlen(text1), &bytesWritten, NULL);

			char const * text2 = Os::getShortProgramName();
			WriteFile(file, text2, strlen(text2), &bytesWritten, NULL);

			char text3[] = " ";
			WriteFile(file, text3, strlen(text3), &bytesWritten, NULL);

			char const * text4 = ApplicationVersion::getInternalVersion();
			WriteFile(file, text4, strlen(text4), &bytesWritten, NULL);

			char text5[] = "\n\n\n";
			WriteFile(file, text5, strlen(text5), &bytesWritten, NULL);

			if (exceptionPointers->ExceptionRecord->ExceptionCode == 0x80000003)
			{
				// write out the fatal buffer
				char const * text6 = FatalNamespace::ms_buffer;
				WriteFile(file, text6, strlen(text6), &bytesWritten, NULL);
			}
			else
			{
				char const * text6 = buffer;
				WriteFile(file, text6, strlen(text6), &bytesWritten, NULL);
			}

			char text7[] = "\n\n";
			WriteFile(file, text7, strlen(text7), &bytesWritten, NULL);

			char const * text8 = "";
			for (int i = 0; text8; ++i)
			{
				text8 = CrashReportInformation::getEntry(i);
				if (text8)
				{
					int const text8Length = strlen(text8);
					if (text8Length)
						WriteFile(file, text8, text8Length, &bytesWritten, NULL);
				}
			}

			CloseHandle(file);
		}

		sprintf(fileName, "%s-%s-%I64d.mdmp", Os::getShortProgramName(), ApplicationVersion::getInternalVersion(), timestamp);
		OutputDebugString("Generating minidump ");
		OutputDebugString(fileName);
		OutputDebugString("\n");
		DebugHelp::writeMiniDump(fileName, exceptionPointers);

		sprintf(fileName, "%s-%s-%I64d.log", Os::getShortProgramName(), ApplicationVersion::getInternalVersion(), timestamp);
		TailFileLogObserver::flushAllTailFileLogObservers(fileName);
	}

	// tell the Os not to abort so we can rethrow the exception
	Os::returnFromAbort();

	// Let the ExitChain do its job
	Fatal("ExceptionHandler invoked");

	// rethrow the exception so that the debugger can catch it
	entered = false;
	return EXCEPTION_CONTINUE_SEARCH;  //lint !e527 // Unreachable
}

// ----------------------------------------------------------------------

static void setFatalVersionString()
{
	char buffer[256];
	sprintf(buffer, "%s: %s\n", Os::getShortProgramName(), ApplicationVersion::getInternalVersion());
	FatalSetVersionString(buffer);
}

// ======================================================================
// Install the engine
//
// Remarks:
//
//   The settings in the Data structure will determine which subsystems
//   get initialized.

void SetupSharedFoundation::install(const Data &data)
{
	InstallTimer const installTimer("SetupSharedFoundation::install");

	DEBUG_REPORT_LOG(true, ("SetupSharedFoundation::install: version %s\n", ApplicationVersion::getInternalVersion()));

	ms_writeMiniDumps = data.writeMiniDumps;
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	// and get the command line stuff in quick so we can make decisions based on the command line settings
	CommandLine::install();

	// feed CommandLine with appropriate strings
	if (data.commandLine)
		CommandLine::absorbString(data.commandLine);
	if (data.argc)
		CommandLine::absorbStrings(const_cast<const char**>(data.argv+1), data.argc-1);

	// load the config file
	ConfigFile::install();
	if (data.configFile)
		IGNORE_RETURN(ConfigFile::loadFile(data.configFile));

	// get the post command-line text for the ConfigFile (key-value pairs)
	const char *configString = CommandLine::getPostCommandLineString();
	if (configString)
	{
		IGNORE_RETURN(ConfigFile::loadFromCommandLine(configString));
	}

	// @todo codereorg should this be here?
	MemoryManager::registerDebugFlags();
#if PRODUCTION == 0
	Profiler::registerDebugFlags();
#endif
	FatalInstall();

	// @todo move these, it's part of sharedDebug.  However, sharedDebug is installed before sharedFoundation, but these need the ConfigFile.  ugh.
#if PRODUCTION == 0
	DebugMonitor::install();
#endif
	SetWarningStrictFatal(ConfigFile::getKeyBool("SharedDebug", "strict", false));

	{
		ConfigSharedFoundation::Defaults defaults;
		defaults.frameRateLimit = data.frameRateLimit;
		defaults.demoMode       = data.demoMode;
		defaults.verboseWarnings = data.verboseWarnings;
		ConfigSharedFoundation::install(defaults);

		if (ConfigSharedFoundation::getCauseAccessViolation())
			static_cast<int*>(0)[0] = 0;
	}

	// @todo codereorg should this be here?
#ifdef _DEBUG
	MemoryManager::setReportAllocations (ConfigSharedFoundation::getMemoryManagerReportAllocations ());
#endif

	MemoryBlockManager::install (ConfigSharedFoundation::getMemoryBlockManagerDebugDumpOnRemove ());

	ExitChain::install();
	Report::install();
	Clock::install(data.clockUsesSleep, data.clockUsesRecalibrationThread);
	CrashReportInformation::install();
	RegistryKey::install(data.productRegistryKey);

	PersistentCrcString::install();
	CrcLowerString::install();

	WatchedByList::install();

	if (data.createWindow)
	{
		DEBUG_FATAL(data.useWindowHandle, ("exactly one of createWindow and useWindowHandle must be true"));
		Os::install(data.hInstance, data.windowName, data.windowNormalIcon, data.windowSmallIcon);
	}
	else
	{
		if (data.useWindowHandle)
			Os::install(data.windowHandle, data.processMessagePump);
		else
			Os::install();
	}

	StaticCallbackEntry::install();

	setFatalVersionString();
}

// ----------------------------------------------------------------------
// Call a function with appropriate exception handling
//
// Remarks:
//
//   If exception handling has been disabled in the config file, this routine
//   will call the callback without exception handling.

void SetupSharedFoundation::callbackWithExceptionHandling(
	void (*callback)(void)   // Routine to call with exception handling
	)
{
	callback();
}

// ----------------------------------------------------------------------
// Uninstall the engine
//
// Remarks:
//
//   This routine will properly uninstall the engine componenets that were
//   installed by SetupSharedFoundation::install().

void SetupSharedFoundation::remove(void)
{
	ExitChain::quit();

	if (!ConfigSharedFoundation::getDemoMode() && GetNumberOfWarnings())
		REPORT(true, Report::RF_print | Report::RF_log | Report::RF_dialog, ("%d warnings logged", GetNumberOfWarnings()));
}

// ======================================================================

SetupSharedFoundation::Data::Data(Defaults defaults)
{
	Zero(*this);

	switch (defaults)
	{
		case D_console: setupConsoleDefaults(); break;
		case D_game:    setupGameDefaults();    break;
		case D_mfc:     setupMfcDefaults();     break;
		default: DEBUG_FATAL(true, ("invalid enum value"));
	}
}

// ----------------------------------------------------------------------

void SetupSharedFoundation::Data::setupGameDefaults()
{
	createWindow                             = true;
	windowName                               = NULL;
	windowNormalIcon                         = NULL;
	windowSmallIcon                          = NULL;
	hInstance                                = NULL;
	useWindowHandle                          = false;
	processMessagePump                       = true;
	windowHandle                             = NULL;
	clockUsesSleep                           = false;
	clockUsesRecalibrationThread             = true;
	writeMiniDumps	                         = false;

	commandLine                              = NULL;
	argc                                     = 0;
	argv                                     = NULL;

	configFile                               = NULL;

	productRegistryKey                       = NULL;

	frameRateLimit                           = CONST_REAL(0);

	lostFocusCallback                        = NULL;

	demoMode                                 = false;
	verboseWarnings                          = false;
}

// ----------------------------------------------------------------------

void SetupSharedFoundation::Data::setupConsoleDefaults()
{
	createWindow                             = false;
	windowName                               = NULL;
	windowNormalIcon                         = NULL;
	windowSmallIcon                          = NULL;
	hInstance                                = NULL;
	useWindowHandle                          = false;
	processMessagePump                       = true;
	windowHandle                             = NULL;
	clockUsesSleep                           = false;
	clockUsesRecalibrationThread             = false;
	writeMiniDumps	                         = false;

	commandLine                              = NULL;
	argc                                     = NULL;
	argv                                     = NULL;

	configFile                               = NULL;

	productRegistryKey                       = NULL;

	frameRateLimit                           = CONST_REAL(0);

	lostFocusCallback                        = NULL;

	demoMode                                 = false;
	verboseWarnings                          = false;
}

// ----------------------------------------------------------------------

void SetupSharedFoundation::Data::setupMfcDefaults()
{
	createWindow                             = false;
	windowName                               = NULL;
	windowNormalIcon                         = NULL;
	windowSmallIcon                          = NULL;
	hInstance                                = NULL;
	useWindowHandle                          = false;
	processMessagePump                       = true;
	windowHandle                             = NULL;
	clockUsesSleep                           = false;
	clockUsesRecalibrationThread             = false;
	writeMiniDumps	                         = false;

	commandLine                              = NULL;
	argc                                     = 0;
	argv                                     = NULL;

	configFile                               = NULL;

	productRegistryKey                       = NULL;

	frameRateLimit                           = CONST_REAL(0);

	demoMode                                 = false;
	verboseWarnings                          = false;
}

// ======================================================================
