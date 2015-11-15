// ======================================================================
//
// StringCompare.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StringCompare_H
#define INCLUDED_StringCompare_H

// ======================================================================

#include <cstring>

// ======================================================================

/// A functor used to allow storing of char* in maps with value lookups
class StringCompare
{
public:
	inline bool operator() (const char*, const char*) const;
};

inline bool StringCompare::operator() (const char *lhs, const char *rhs) const
{
	return static_cast<bool>(strcmp(lhs, rhs) < 0);
}

// ======================================================================

/// A functor used to allow storing of case insensitive char* in maps with value lookups
class StringCaseCompare
{
public:
	inline bool operator() (const char*, const char*) const;
};

inline bool StringCaseCompare::operator() (const char *lhs, const char *rhs) const
{
	return static_cast<bool>(_stricmp(lhs, rhs) < 0);
}

// ======================================================================

#endif
