// ======================================================================
//
// ConfigSharedLog.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConfigSharedLog_H
#define INCLUDED_ConfigSharedLog_H

// ======================================================================

class ConfigSharedLog
{
public:
	static void install();

	static int getLogNetQueueSize();
	static int getLogNetReconnectTimeMs();
	static bool getLogReportLogs();
	static bool getLogReportWarnings();
	static bool getLogReportFatals();
	static bool getLogStderr();
	static int getNumberOfLogTargets();
	static char const * getLogTarget(int index);
};

// ======================================================================

#endif // INCLUDED_ConfigSharedLog_H

