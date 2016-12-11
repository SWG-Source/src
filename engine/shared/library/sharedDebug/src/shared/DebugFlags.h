// ======================================================================
//
// DebugFlags.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DebugFlags_H
#define INCLUDED_DebugFlags_H

// ======================================================================

#include "CallStack.h"

// ======================================================================

class DebugFlags
{
	friend class Os;

public:

	typedef void (*ReportRoutine1)();
	typedef void (*ReportRoutine2)(void *context);

public:

	static DLLEXPORT void registerFlag(bool &variable, const char *section, const char *name);
	static DLLEXPORT void registerFlag(bool &variable, const char *section, const char *name, ReportRoutine1 reportRoutine, int reportPriority=0);
	static void           registerFlag(bool &variable, const char *section, const char *name, ReportRoutine2 reportRoutine, void *context, int reportPriority=0);
	static DLLEXPORT void unregisterFlag(bool &variable);

	static void callReportRoutines();

	static bool *findFlag(const char *section, const char *name);

	static int getNumberOfFlags();
	static char const * getFlagSection(int index);
	static char const * getFlagName(int index);

public:

	struct Flag
	{
		bool           *variable;
		const char     *section;
		const char     *name;
		int             reportPriority;
		ReportRoutine1  reportRoutine1;
		ReportRoutine2  reportRoutine2;
		void           *context;
#ifdef _DEBUG
		CallStack m_callStack;
#endif
	};

private:

	static void config(bool &variable, const char *configSection, const char *configName);
	static void insert(const Flag &flag);
	static Flag const * getFlag(char const * const section, char const * name);

private:

	typedef std::vector<Flag> FlagVector;

	static FlagVector ms_flagsSortedByName;
	static FlagVector ms_flagsSortedByReportPriority;
};

// ======================================================================

#endif
