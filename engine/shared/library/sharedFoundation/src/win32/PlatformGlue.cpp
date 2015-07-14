// ======================================================================
//
// PlatformGlue.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/PlatformGlue.h"

#include <cstdio>
#include <cstdarg>

// ======================================================================

namespace
{

// ----------------------------------------------------------------------

class GmTimeSection
{
public:

	GmTimeSection()
	{
		InitializeCriticalSection(&m_section);
	}

	~GmTimeSection()
	{
		DeleteCriticalSection(&m_section);
	}

	void enter()
	{
		EnterCriticalSection(&m_section);
	}

	void leave()
	{
		LeaveCriticalSection(&m_section);
	}

private:
	CRITICAL_SECTION m_section;
};

// ----------------------------------------------------------------------

static GmTimeSection s_gmTimeSection;

// ----------------------------------------------------------------------

}

// ======================================================================
// Tokenizing a string.
// Like strtok, except re-entrant.

char *strsep(char **string, const char *delim)
{
	char *result = *string;

	// handle no string specified, or the end of the string
	if (result == 0)
		return 0;

	// skip leading delimiters
	result = result + strspn(result, delim);

	// handle trailing delimiters
	if (*result == '\0')
		return 0;

	// look for the first delimiter
	const int len = strcspn(result, delim);
	if (result[len] == '\0')
	{
		// hit the end of the string
		*string = 0;
	}
	else
	{
		// terminate the string and return the substring
		result[len] = '\0';
		*string = result + len + 1;
	}

	return result;
}

// ----------------------------------------------------------------------

int snprintf(char *buffer, size_t count, const char *format, ...)
{
	va_list va;

	va_start(va, format);
		const int result = _vsnprintf(buffer, count, format, va);
	va_end(va);

	return result;
}

// ----------------------------------------------------------------------

struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	s_gmTimeSection.enter();
		tm *t = gmtime(timep);
		*result = *t;
	s_gmTimeSection.leave();
	return result;
}

// ----------------------------------------------------------------------

int finite(double value)
{
	return _finite(value);
}

// ======================================================================
