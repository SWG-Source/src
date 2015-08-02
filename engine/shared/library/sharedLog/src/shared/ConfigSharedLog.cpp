// ======================================================================
//
// ConfigSharedLog.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedLog/ConfigSharedLog.h"

#include "sharedFoundation/ConfigFile.h"

#include <vector>

// ======================================================================

namespace ConfigSharedLogNamespace
{
	typedef std::vector<char const *> StringPtrArray;
	int ms_logNetQueueSize;
	int ms_logNetReconnectTimeMs;
	bool ms_logReportLogs;
	bool ms_logReportWarnings;
	bool ms_logReportFatals;
	bool ms_logStderr;
	StringPtrArray ms_logTargets; // ConfigFile owns the pointer
}
using namespace ConfigSharedLogNamespace;

#define KEY_BOOL(a,b)   (ms_ ## a = ConfigFile::getKeyBool("SharedLog", #a, b))
#define KEY_INT(a,b)    (ms_ ## a = ConfigFile::getKeyInt("SharedLog", #a, b))

// ======================================================================

void ConfigSharedLog::install()
{
	KEY_INT (logNetQueueSize,   2000);
	KEY_INT (logNetReconnectTimeMs, 10000);
	KEY_BOOL(logReportLogs,     false);
	KEY_BOOL(logReportWarnings, false);
	KEY_BOOL(logReportFatals,   true);
	KEY_BOOL(logStderr,         false);

	int index = 0;
	char const * result = 0;
	do
	{
		result = ConfigFile::getKeyString("SharedLog", "logTarget", index++, 0);
		if (result)
			ms_logTargets.push_back(result);
	}
	while (result);
}

// ----------------------------------------------------------------------

int ConfigSharedLog::getLogNetQueueSize()
{
	return ms_logNetQueueSize;
}

// ----------------------------------------------------------------------

int ConfigSharedLog::getLogNetReconnectTimeMs()
{
	return ms_logNetReconnectTimeMs;
}

// ----------------------------------------------------------------------

bool ConfigSharedLog::getLogReportLogs()
{
	return ms_logReportLogs;
}

// ----------------------------------------------------------------------

bool ConfigSharedLog::getLogReportWarnings()
{
	return ms_logReportWarnings;
}

// ----------------------------------------------------------------------

bool ConfigSharedLog::getLogReportFatals()
{
	return ms_logReportFatals;
}

// ----------------------------------------------------------------------

bool ConfigSharedLog::getLogStderr()
{
	return ms_logStderr;
}

// ----------------------------------------------------------------------

int ConfigSharedLog::getNumberOfLogTargets()
{
	return static_cast<int>(ms_logTargets.size());
}

// ----------------------------------------------------------------------

char const * ConfigSharedLog::getLogTarget(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfLogTargets());
	return ms_logTargets[static_cast<size_t>(index)];
}

// ======================================================================

