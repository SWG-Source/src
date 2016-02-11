// ======================================================================
//
// DebugFlags.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DebugFlags.h"

#include "sharedFoundation/ConfigFile.h"

#include <algorithm>
#include <vector>

// ======================================================================

std::vector<DebugFlags::Flag> DebugFlags::ms_flagsSortedByName;
std::vector<DebugFlags::Flag> DebugFlags::ms_flagsSortedByReportPriority;

// ======================================================================
/** 
 * @internal
 */

void DebugFlags::config(bool &variable, const char *configSection, const char *configName)
{
	DEBUG_FATAL(strchr(configSection, ' ') != nullptr, ("no spaces are allowed in debug flag section names: %s", configSection));
	DEBUG_FATAL(strchr(configName, ' ') != nullptr, ("no spaces are allowed in debug flag names: %s", configName));

	if (configSection && configName && ConfigFile::isInstalled())
	{
#ifdef _DEBUG
		Flag const * const flag = getFlag(configSection, configName);
		if (flag)
		{
			DEBUG_WARNING(true, ("DebugFlags::registerFlag: %s/%s is already registered", configSection, configName));
			flag->m_callStack.debugLog();
		}
#endif

		variable = ConfigFile::getKeyBool(configSection, configName, variable);
	}
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

static bool sortByNameLess(const DebugFlags::Flag &lhs, const DebugFlags::Flag &rhs)
{
	const int s = strcmp(lhs.section, rhs.section);
	if (s < 0)
		return true;
	if (s > 0)
		return false;

	return strcmp(lhs.name, rhs.name) < 0;
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

static bool sortByReportPriorityLess(const DebugFlags::Flag &lhs, const DebugFlags::Flag &rhs)
{
	return (lhs.reportPriority < rhs.reportPriority);
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

void DebugFlags::insert(const Flag &flag)
{
	ms_flagsSortedByName.insert(std::lower_bound(ms_flagsSortedByName.begin(), ms_flagsSortedByName.end(), flag, sortByNameLess), flag);
	if (flag.reportRoutine1 || flag.reportRoutine2)
		ms_flagsSortedByReportPriority.insert(std::lower_bound(ms_flagsSortedByReportPriority.begin(), ms_flagsSortedByReportPriority.end(), flag, sortByReportPriorityLess), flag);
}

// ----------------------------------------------------------------------
/**
 * Register a debug flag.
 * The parameters passed to this function must never go out of scope.
 * @param variable The bool variable to determine whether the flag is on or not.
 * @param section  The section for the variable.
 * @param name     The name of the variable.
 */

void DebugFlags::registerFlag(bool &variable, const char *section, const char *name)
{
	NOT_NULL(section);
	NOT_NULL(name);

	config(variable, section, name);

	Flag f;
	f.variable = &variable;
	f.section = section;
	f.name = name;
	f.reportPriority = 0;
	f.reportRoutine1 = nullptr;
	f.reportRoutine2 = nullptr;
	f.context = nullptr;
#ifdef _DEBUG
	f.m_callStack.sample();
#endif

	insert(f);
}

// ----------------------------------------------------------------------

void DebugFlags::registerFlag(bool &variable, const char *section, const char *name, ReportRoutine1 reportRoutine, int reportPriority)
{
	NOT_NULL(section);
	NOT_NULL(name);

	config(variable, section, name);

	Flag f;
	f.variable = &variable;
	f.section = section;
	f.name = name;
	f.reportPriority = reportPriority;
	f.reportRoutine1 = reportRoutine;
	f.reportRoutine2 = nullptr;
	f.context = nullptr;
#ifdef _DEBUG
	f.m_callStack.sample();
#endif

	insert(f);
}

// ----------------------------------------------------------------------

void DebugFlags::registerFlag(bool &variable, const char *section, const char *name, ReportRoutine2 reportRoutine, void *context, int reportPriority)
{
	NOT_NULL(section);
	NOT_NULL(name);

	config(variable, section, name);

	Flag f;
	f.variable = &variable;
	f.section = section;
	f.name = name;
	f.reportPriority = reportPriority;
	f.reportRoutine1 = nullptr;
	f.reportRoutine2 = reportRoutine;
	f.context = context;
#ifdef _DEBUG
	f.m_callStack.sample();
#endif

	insert(f);
}

// ----------------------------------------------------------------------
/**
 * Unregister a debug flag.
 */

void DebugFlags::unregisterFlag(bool &variable)
{
	{
		FlagVector::iterator end = ms_flagsSortedByName.end();
		for (FlagVector::iterator i = ms_flagsSortedByName.begin(); i != end; ++i)
			if (i->variable == &variable)
			{
				ms_flagsSortedByName.erase(i);
				break;
			}
	}

	{
		FlagVector::iterator end = ms_flagsSortedByReportPriority.end();
		for (FlagVector::iterator i = ms_flagsSortedByReportPriority.begin(); i != end; ++i)
			if (i->variable == &variable)
			{
				ms_flagsSortedByReportPriority.erase(i);
				break;
			}
	}
}

// ----------------------------------------------------------------------

void DebugFlags::callReportRoutines()
{
	FlagVector::const_iterator end = ms_flagsSortedByReportPriority.end();
	for (FlagVector::const_iterator i = ms_flagsSortedByReportPriority.begin(); i != end; ++i)
	{
		const Flag &f = *i;
		if (*f.variable)
		{
			if (f.reportRoutine1)
				f.reportRoutine1();
			else
				if (f.reportRoutine2)
					f.reportRoutine2(f.context);
		}
	}
}

// ----------------------------------------------------------------------

bool * DebugFlags::findFlag(char const * const section, char const * const name)
{
	Flag const * const flag = getFlag(section, name);
	if (flag)
		return flag->variable;

	return 0;
}

// ----------------------------------------------------------------------

DebugFlags::Flag const * DebugFlags::getFlag(char const * const section, char const * name)
{
	FlagVector::const_iterator iEnd = ms_flagsSortedByName.end();
	for (FlagVector::const_iterator i = ms_flagsSortedByName.begin(); i != iEnd; ++i)
	{
		Flag const & f = *i;
		if (strcmp(f.section, section) == 0 && strcmp(f.name, name) == 0)
			return &f;
	}

	return nullptr;
}

// ----------------------------------------------------------------------

int DebugFlags::getNumberOfFlags()
{
	return static_cast<int>(ms_flagsSortedByName.size());
}

// ----------------------------------------------------------------------

char const * DebugFlags::getFlagSection(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfFlags());
	return ms_flagsSortedByName[index].section;
}

// ----------------------------------------------------------------------

char const * DebugFlags::getFlagName(int const index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfFlags());
	return ms_flagsSortedByName[index].name;
}

// ======================================================================
