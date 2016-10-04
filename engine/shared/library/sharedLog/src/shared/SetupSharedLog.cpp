// ======================================================================
//
// SetupSharedLog.cpp
//
// Copyright 2002-2003 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedLog/SetupSharedLog.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/ConfigSharedLog.h"
#include "sharedLog/FileLogObserver.h"
#include "sharedLog/NetLogObserver.h"
#include "sharedLog/LogManager.h"
#include "sharedLog/Log.h"
#include "sharedLog/TailFileLogObserver.h"

// ======================================================================

namespace SetupSharedLogNamespace
{
	void logReportLog(char const *message);
	void logReportWarning(char const *message);
	void logReportFatal(char const *message);

	bool              ms_installed;
	std::string const cms_reportLogChannel("reportLog");
	std::string const cms_reportWarningChannel("warning");
	std::string const cms_reportFatalChannel("fatal");
}
using namespace SetupSharedLogNamespace;

// ======================================================================

void SetupSharedLogNamespace::logReportLog(char const *message)
{
	LOG(cms_reportLogChannel, ("%s", message));
}

// ----------------------------------------------------------------------

void SetupSharedLogNamespace::logReportWarning(char const *message)
{
	LOG(cms_reportWarningChannel, ("%s", message));
}

// ----------------------------------------------------------------------

void SetupSharedLogNamespace::logReportFatal(char const *message)
{
	LOG(cms_reportFatalChannel, ("%s", message));
}

// ======================================================================

void SetupSharedLog::install(std::string const &procId, bool flushOnWrite)
{
	InstallTimer const installTimer("SetupSharedLog::install");

	DEBUG_FATAL (ms_installed, ("SetupSharedLog::install already installed"));
	ms_installed = true;

	ConfigSharedLog::install();
	LogManager::install(procId, flushOnWrite);
	FileLogObserver::install();
	if (procId.compare("LogServer") != 0) {
		NetLogObserver::install();
	}
	TailFileLogObserver::install();

	if (ConfigSharedLog::getLogReportLogs())
		Report::bindLogCallback(logReportLog);

	if (ConfigSharedLog::getLogReportWarnings())
		Report::bindWarningCallback(logReportWarning);

	if (ConfigSharedLog::getLogReportFatals())
		Report::bindFatalCallback(logReportFatal);
}

// ----------------------------------------------------------------------

void SetupSharedLog::remove()
{
	DEBUG_FATAL (!ms_installed, ("SetupSharedLog::remove not installed"));
	ms_installed = false;
}

// ======================================================================

