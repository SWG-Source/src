// ======================================================================
//
// vsnprintf.cpp
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/vsnprintf.h"

#include <cstdio>

// ======================================================================
// Format a printf-style string into a text buffer of fixed size
//
// Return value:
//
//   The number of characters written into the buffer, or -1 if the buffer was too small
//
// Remarks:
//
//   If the buffer would overflow, the nullptr terminating character is not written and -1
//   will be returned.

#ifdef _MSC_VER

int vsnprintf(char *buffer, size_t count, const char *format, va_list va)
{
	return _vsnprintf(buffer, count, format, va);
}

#endif
// ======================================================================
