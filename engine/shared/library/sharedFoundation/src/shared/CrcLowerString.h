// ======================================================================
//
// CrcLowerString.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
// 
// ======================================================================

#ifndef INCLUDED_CrcLowerString_H
#define INCLUDED_CrcLowerString_H

// ======================================================================

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/PersistentCrcString.h"

class MemoryBlockManager;

// ======================================================================
/**
 * The CrcLowerString class is designed to permit faster case-insensitive
 * string searching.  The strings are first compared using CRC, and then
 * compared using a lowercasing strcmp() call only if the CRC's match.
 *
 * Note that sorting strings by the return value of compare() will not
 * sorted lexiographic manner, because the strings are first compared by
 * their CRC which does not indicate lexiographic ordering of the strings.
 * Even so, CRC strings may still be stored and searched upon by various
 * high-performance search algorithms like a binary searches.
 */

class CrcLowerString : public PersistentCrcString
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessPtrComparator
	{
		//-- returns true if a is less than b
		bool operator ()(const CrcLowerString * a, const CrcLowerString * b) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Provide a predicate that compares CrcLowerString instances based on
	 * the underlying string lexicographical comparison (case sensitive).
	 *
	 * The standard CrcLowerString operator < compares efficiently based
	 * on the Crc value.  Typically this is what you want, for presumably
	 * the user is using CrcLowerString instances to speed up comparisons.
	 * However, in the case where you want to sort CrcLowerString instances
	 * based on the lexicographical (ABC) ordering, use this function object.
	 * Sorts identically to strcmp().
	 *
	 * Can be used as a predicate object (functor) in STL routines.
	 */

	struct LessAbcOrderReferenceComparator
	{
		bool operator ()(const CrcLowerString &a, const CrcLowerString &b) const;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static uint32 calculateCrc(const char *newString);

public:

	CrcLowerString();
	explicit CrcLowerString(const char *newString);
	CrcLowerString(const char *newString, uint32 newCrc);
	CrcLowerString(const CrcLowerString &rhs);
	virtual ~CrcLowerString(void);

	void         setString(const char *newString);

	int          compare(const char *otherString, uint32 otherCrc) const;
	int          compare(const CrcLowerString &otherString) const;

	CrcLowerString &operator =(const CrcLowerString &rhs);

public:

	/**
	 * A CrcLowerString representation of the empty string (i.e. "").
	 */
	static const CrcLowerString  empty;

protected:

	CrcLowerString(const char *newString, PersistentCrcString::ConstChar);
};

// ======================================================================
// class CrcLowerString::LessPtrComparator
// ======================================================================

inline bool CrcLowerString::LessPtrComparator::operator () (const CrcLowerString * a, const CrcLowerString * b) const
{
	return *a < *b;
}

// ======================================================================
// class CrcLowerString::LessAbcOrderReferenceComparator
// ======================================================================

inline bool CrcLowerString::LessAbcOrderReferenceComparator::operator ()(const CrcLowerString &a, const CrcLowerString &b) const
{
	return (strcmp(a.getString(), b.getString()) < 0);
}

// ======================================================================

#endif
