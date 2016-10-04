// ======================================================================
//
// Fatal.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Fatal.h"

#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugHelp.h"
#include "sharedDebug/PixCounter.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedSynchronization/Mutex.h"

#include <cstdio>
#include <string>

// ======================================================================

namespace FatalNamespace
{
	Mutex ms_criticalSection;
	char  ms_buffer[32 * 1024];
	bool  ms_throwExceptions;
	int   ms_numberOfWarnings = 0;
	bool  ms_strict = false;

	WarningCallback s_warningCallback = nullptr;

#if PRODUCTION == 0
	PixCounter::ResetInteger ms_numberOfWarningsThisFrame;
#endif
}
using namespace FatalNamespace;

// ======================================================================

void FatalInstall()
{
#if PRODUCTION == 0
	ms_numberOfWarningsThisFrame.bindToCounter("Warnings");
#endif
}

// ======================================================================

static void formatMessage(char *buffer, int bufferLength, int stackDepth, const char *type, const char *format, va_list va)
{
	const int callStackOffset = 4;
	const int maxStackDepth   = 64;

	if (stackDepth > maxStackDepth)
		stackDepth = maxStackDepth;

	// look up the call stack information
	const int callStackSize = callStackOffset + stackDepth;
	uint32 callStack[callStackOffset + maxStackDepth];

	// allow complete disable of the call stack lookup
	if (stackDepth < 0)
		memset(callStack, 0, sizeof(callStack));
	else
		DebugHelp::getCallStack(callStack, callStackOffset + stackDepth);

	// make sure the buffer is always nullptr terminated
	buffer[--bufferLength] = '\0';

	// look up the caller's file and line
	if (callStack[callStackOffset])
	{
		char lib[4 * 1024] = { '\0' };
		char file[4 * 1024] = { '\0' };
		int line = 0;
		if (ConfigSharedFoundation::getLookUpCallStackNames() && DebugHelp::lookupAddress(callStack[callStackOffset], lib, file, sizeof(file), line))
			snprintf(buffer, bufferLength, "%s(%d) : %s %08x: \n", file, line, type, static_cast<int>(Crc::calculate(format)));
		else
			snprintf(buffer, bufferLength, "(0x%08X) : %s %08x: \n", static_cast<int>(callStack[callStackOffset]), type, static_cast<int>(Crc::calculate(format)));
	}
	else
	{
		snprintf(buffer, bufferLength, "    (%08x): ", static_cast<int>(Crc::calculate(format)));
	}

	{
		const int length = strlen(buffer);
		buffer += length;
		bufferLength -= length;
	}

	// add the user message
	vsnprintf(buffer, bufferLength, format, va);

	{
		const int length = strlen(buffer);
		buffer += length;
		bufferLength -= length;
	}

	// add the newline
	if (bufferLength)
	{
		*buffer = '\n';
		++buffer;
		--bufferLength;
		*buffer = '\0';
	}

	if (stackDepth >= 0)
	{
		// add the call stack
		for (int i = callStackOffset+1; i < callStackSize; ++i)
			if (callStack[i])
			{
				char lib[1024];
				char file[1024];
				int line = 0;

				if (ConfigSharedFoundation::getLookUpCallStackNames() && DebugHelp::lookupAddress(callStack[i], lib, file, sizeof(file), line))
					snprintf(buffer, bufferLength, "   %s(%d) : caller %d\n", file, line, i-callStackOffset);
				else
					snprintf(buffer, bufferLength, "   (0x%08X) : caller %d\n", static_cast<int>(callStack[i]), i-callStackOffset);

				const int length = strlen(buffer);
				buffer += length;
				bufferLength -= length;
			}
	}

	if (bufferLength == 0)
	{
		buffer[-2] = '+';
		buffer[-1] = '\n';
	}
}

// ----------------------------------------------------------------------
/**
 * Terminate the game due to an abnormal condition.
 *
 * This routine does not return.
 *
 * This routine will call ExitChain::fatal() to clean up the system as appropriate.
 *
 * @param format  printf-style formatting string
 * @see PreFatal()
 */

static void InternalFatal(const char *format, va_list va)
{
	// if the per-thread-data isn't installed, then we know we're single-threaded and can use the static buffer
	ms_criticalSection.enter();
		const int bufferSize = sizeof(ms_buffer);
		const int length = strlen(ms_buffer);
		formatMessage(ms_buffer+length, bufferSize-length, ConfigSharedFoundation::getFatalCallStackDepth(), "FATAL", format, va);
	ms_criticalSection.leave();

	REPORT(true, Report::RF_print | Report::RF_log | Report::RF_fatal, ("%s", ms_buffer));

	IGNORE_RETURN(Os::copyTextToClipboard(ms_buffer));

	if (ms_throwExceptions)
		throw FatalException(ms_buffer, FatalException::ZeroSourceString); //lint !e1549 // Function not declared to throw

#ifdef _WIN32
	{
		__asm int 3;
	}
#endif

	DEBUG_OUTPUT_CHANNEL("Foundation\\Fatal", ("%s", ms_buffer));
	ExitChain::fatal();
	REPORT(true, Report::RF_fatal | Report::RF_dialog, ("%s", ms_buffer));
	Os::abort();
}

// ----------------------------------------------------------------------

static void InternalWarning(const char *format, int extraFlags, va_list va, int stackDepth = ConfigSharedFoundation::getWarningCallStackDepth())
{
#if PRODUCTION == 0
	++ms_numberOfWarningsThisFrame;
#endif

	char buffer[4 * 1024];

	if (nullptr != s_warningCallback)
	{
		strcpy(buffer, "WARNING: ");
		vsnprintf(buffer + 9, sizeof(buffer) - 9, format, va);
		strcat(buffer, "\n");
		s_warningCallback(buffer);
	}

	formatMessage(buffer, sizeof(buffer), stackDepth, "WARNING", format, va);

#ifdef _DEBUG
	if (DataLint::isInstalled())
	{
		DataLint::logWarning(buffer);
		return;
	}
#endif

	++ms_numberOfWarnings;
	REPORT(true, Report::RF_print | Report::RF_log | Report::RF_warning | extraFlags, ("%s", buffer));
	DEBUG_OUTPUT_CHANNEL("Foundation\\Warning", ("%s", buffer));
}

//----------------------------------------------------------------------

void SetWarningCallback(WarningCallback warningCallback)
{
	s_warningCallback = warningCallback;
}

// ======================================================================

FatalException::FatalException(char *newMessage, Zero zero)
: message(DuplicateString(newMessage))
{
	UNREF(zero);
	if (newMessage)
		newMessage[0] = '\0';
}

// ----------------------------------------------------------------------

FatalException::FatalException(const FatalException &fatalException)
: message(DuplicateString(fatalException.message))
{
}

// ----------------------------------------------------------------------

FatalException::~FatalException(void)
{
	delete [] message;
}

// ----------------------------------------------------------------------

FatalException &FatalException::operator =(const FatalException &fatalException)
{
	if (&fatalException != this)
	{
		delete [] message;
		message = DuplicateString(fatalException.message);
	}

	return *this;
}

// ----------------------------------------------------------------------

const char *FatalException::getMessage(void) const
{
	return message;
}

// ======================================================================

void FatalSetVersionString(const char *string)
{
	strcat(ms_buffer, string);
}

// ----------------------------------------------------------------------

void FatalSetThrowExceptions(bool throwExceptions)
{
	ms_throwExceptions = throwExceptions;
}

// ----------------------------------------------------------------------

int GetNumberOfWarnings()
{
	return ms_numberOfWarnings;
}

// ----------------------------------------------------------------------

void Fatal(const char *format, ...)
{
	va_list va;
	va_start(va, format);

		InternalFatal(format, va);

	va_end(va);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void DebugFatal(const char *format, ...)
{
	va_list va;
	va_start(va, format);

		InternalFatal(format, va);

	va_end(va);
}

#endif

// ----------------------------------------------------------------------

void Warning(const char *format, ...)
{
	va_list va;
	va_start(va, format);

		InternalWarning(format, 0, va);

	va_end(va);
}

// ----------------------------------------------------------------------

void WarningStackDepth(int stackDepth, const char *format, ...)
{
	va_list va;
	va_start(va, format);

	InternalWarning(format, 0, va, stackDepth);

	va_end(va);
}

// ----------------------------------------------------------------------

void ConsoleWarning(const char *format, ...)
{
	va_list va;
	va_start(va, format);

		InternalWarning(format, Report::RF_console, va);

	va_end(va);
}

// ----------------------------------------------------------------------

void SetWarningStrictFatal(bool strict)
{
	ms_strict = strict;
}

// ----------------------------------------------------------------------

bool GetWarningStrictFatal()
{
	return ms_strict;
}

// ----------------------------------------------------------------------

void WarningStrictFatal(const char *format, ...)
{
	va_list va;
	va_start(va, format);

		if (ms_strict)
			InternalFatal(format, va);
		else
			InternalWarning(format, 0, va);

	va_end(va);
}

// ======================================================================
