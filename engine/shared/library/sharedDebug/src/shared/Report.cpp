// ======================================================================
//
// Report.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/Report.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugMonitor.h"
#include "sharedFoundation/FloatingPointUnit.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/Production.h"

#include <cstdio>
#include <cstdarg>

// ======================================================================

namespace ReportNamespace
{
	Report::Callback ms_logCallback;
	Report::Callback ms_warningCallback;
	Report::Callback ms_fatalCallback;
	bool             ms_logAllReports;
	int              ms_flags;
};
using namespace ReportNamespace;

// ======================================================================

void Report::install()
{
	DebugFlags::registerFlag(ms_logAllReports, "SharedDebug", "logAllReports");
}

// ----------------------------------------------------------------------

void Report::bindLogCallback(Callback callback)
{
	ms_logCallback = callback;
}

// ----------------------------------------------------------------------

void Report::bindWarningCallback(Callback callback)
{
	ms_warningCallback = callback;
}

// ----------------------------------------------------------------------

void Report::bindFatalCallback(Callback callback)
{
	ms_fatalCallback = callback;
}

// ======================================================================
/**
 * Setup the debug print flags for the next debug print from this thread
 *
 * This routine should never be called directly, but only through the DEBUG_PRINT macros.
 *
 * @internal
 */

void Report::setFlags(int flags)
{
	if (ms_logAllReports)
		flags |= RF_log;

	if (!PerThreadData::isThreadInstalled())
	{
		// if the per-thread-data isn't installed, then we know we're single-threaded and can use the static flags
		ms_flags = flags;
	}
	else
		PerThreadData::setDebugPrintFlags(flags);
}

// ----------------------------------------------------------------------

void Report::puts(const char *buffer)
{
	int flags;
	if (!PerThreadData::isThreadInstalled())
	{
		// if the per-thread-data isn't installed, then we know we're single-threaded and can use the static flags
		flags = ms_flags;
	}
	else
		flags = PerThreadData::getDebugPrintFlags();

	// handle logging callback functions
	if (flags & RF_fatal)
	{
		if (ms_fatalCallback)
			(*ms_fatalCallback)(buffer);
	}
	else
		if (flags & RF_warning)
		{
			if (ms_warningCallback)
				(*ms_warningCallback)(buffer);
		}
		else
			if (flags & RF_log)
			{
				if (ms_logCallback)
					(*ms_logCallback)(buffer);
			}

	if (flags & RF_print)
	{
#ifdef WIN32
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hStdOut)
		{
			DWORD bytesWritten;
			WriteFile(hStdOut, buffer, strlen(buffer), &bytesWritten, 0);
		}
#else
		fputs(buffer, stdout);
#endif
	}

#if PRODUCTION == 0
	if (flags & RF_print)
	{
		DebugMonitor::print(buffer);
		if (flags & (RF_fatal | RF_dialog))
			DebugMonitor::print("\n");
	}
#endif

	if (flags & RF_log)
	{
		const WORD fp1 = FloatingPointUnit::getControlWord();

#ifdef WIN32
		OutputDebugString(buffer);
		if (flags &	RF_console)
			fputs(buffer, stderr);
#else
		fputs(buffer, stderr);
#endif

		// fatal strings and dialog messages do not have newlines on the end of them, but we want them in the logs
		if (flags & (RF_fatal | RF_dialog))
		{
#ifdef WIN32
			OutputDebugString("\n");
			if (flags &	RF_console)
				fputs("\n", stderr);
#else
			fputs("\n", stderr);
#endif
		}

		const WORD fp2 = FloatingPointUnit::getControlWord();

		// -qq- HACK this is an attempt to work around OutputDebugString resetting the FPU precision to 53 bits when running under the debugger
		if (fp1 != fp2)
			FloatingPointUnit::setControlWord(fp1);
	}

	// fatal strings should be made very obvious, so pop up a message box
	//if ((flags & RF_dialog) && Os::isMainThread())
	//{
	//	const char *title = "Report";
	//	if (flags & RF_fatal)
	//		title = "Fatal Report";

	//	MessageBox(nullptr, buffer, title, MB_OK | MB_ICONEXCLAMATION);
	//}
}

// ----------------------------------------------------------------------
/**
 * Format and print a debugging message.
 *
 * This routine should never be called directly, but only through the REPORT
 * macros.
 *
 * This routine will send the specified string to the DebugMonitor.  It will
 * also be logged it to the debugger if the RF_log enum was specified.  If
 * the RF_fatal flag was specified, the routine will display a message box
 * with the string in it as well.
 *
 * @internal
 */

void Report::vprintf(const char *format, va_list va)
{
	char buffer[8 * 1024];

	// make sure the buffer is always nullptr terminated
	buffer[sizeof(buffer)-1] = '\0';

	// format the string
	IGNORE_RETURN(vsnprintf(buffer, sizeof(buffer)-1, format, va));

	// handle overflow reasonably nicely
	if (strlen(buffer) == sizeof(buffer)-1)
	{
		buffer[sizeof(buffer)-3] = '+';
		buffer[sizeof(buffer)-2] = '\n';
	}
	
	puts(buffer);
}

// ----------------------------------------------------------------------
/**
 * Format and print a debugging message.
 *
 * This routine should never be called directly, but only through the REPORT
 * macros.
 *
 * This routine will send the specified string to the DebugMonitor.  It will
 * also be logged it to the debugger if the RF_log enum was specified.  If
 * the RF_fatal flag was specified, the routine will display a message box
 * with the string in it as well.
 *
 * @internal
 */

void Report::printf(const char *format, ...)
{
	va_list va;

	va_start(va, format);

		vprintf(format, va);

	va_end(va);
}

// ======================================================================
