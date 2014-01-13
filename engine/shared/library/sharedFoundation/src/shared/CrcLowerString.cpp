// ======================================================================
//
// CrcLowerString.cpp
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CrcLowerString.h"

#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"

// ======================================================================

const CrcLowerString CrcLowerString::empty("", PersistentCrcString::CC_true);

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(CrcLowerString, true, 0, 0, 0);

// ======================================================================
// Compute the CRC for a string
//
// Return value:
//
//   The CRC for the specified string
//
// Remarks:
//
//   This routine is provided so that other code can calculate the CRC for
//   a CrcLowerString without having to create a CrcLowerString, enabling the user
//   to call the compare(const char *, uint32) function.
//
//   This routine currently simply calls CRC32::calculateWithToLower() on the specified
//   string.

uint32 CrcLowerString::calculateCrc(const char *newString)
{
	return TemporaryCrcString(newString, true).getCrc();
}

// ======================================================================
/**
 * Construct a new CrcLowerString, initialized with empty string.
 *
 * This function is provided to satisfy the requirements for a
 * std::set<CrcLowerString> and related STL containers.
 */

CrcLowerString::CrcLowerString()
: PersistentCrcString()
{
}

// ----------------------------------------------------------------------
// Construct a new CrcLowerString
//
// Remarks:
//
//   The constructor will copy its argument into local memory, and then
//   call CrcLowerString::calculateCrc() on the string to compute the CRC once.
//
// See Also:
//
//   CrcLowerString::calculateCrc()

CrcLowerString::CrcLowerString(const char *newString)
: PersistentCrcString(newString, true)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a new CrcLowerString.
 * 
 * The constructor will copy its argument into local memory.  It will
 * assume that the specified CRC is correct for the string.
 * 
 * @param newString  String to generate the CRC String from
 * @param newCrc  CRC for the string
 */

CrcLowerString::CrcLowerString(const char *newString, uint32 newCrc)
: PersistentCrcString(newString, newCrc)
{
}

// ----------------------------------------------------------------------
/**
 * Copy construct a new CrcLowerString from an existing CrcLowerString.
 * 
 * The constructor will copy its argument into local memory.
 *
 * This function normally would not be present since there shouldn't
 * be much need to duplicate a crc-based string.  However, the need
 * arose to use a CrcLowerString as a key for a std::map, so the
 * copy constructor was needed.
 * 
 * @param rhs  Existing CrcLowerString to copy
 */

CrcLowerString::CrcLowerString(const CrcLowerString &rhs)
: PersistentCrcString(rhs)
{
}

// ----------------------------------------------------------------------

CrcLowerString::CrcLowerString(const char *newString, PersistentCrcString::ConstChar)
: PersistentCrcString(newString, PersistentCrcString::CC_true)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a CrcLowerString.
 * 
 * This routine will free the resources associated with this CrcLowerString.
 */

CrcLowerString::~CrcLowerString(void)
{
}

// ----------------------------------------------------------------------
/**
 * Set the new string for a CrcLowerString.
 * 
 * This routine will delete the old string and reallocate the new string.
 * It will also recompute the CRC for the string.
 * 
 * @param newString  The new value of the CrcLowerString
 */

void CrcLowerString::setString(const char *newString)
{
	set(newString, true);
}

// ----------------------------------------------------------------------
/**
 * Compare this CrcLowerString with a string and a CRC.
 * 
 * The return value does not indicate the lexiographic ordering of the
 * strings, because they are first compared using the CRC, and only if
 * the CRC's match are they compared by strcmp().
 * 
 * @param otherString  String to compare against
 * @param otherCrc  CRC of the string to compare against
 * @return See Remarks section
 */

int CrcLowerString::compare(const char *otherString, uint32 otherCrc) const
{
	NOT_NULL(otherString);

	const uint32 crc = getCrc();

	if (crc < otherCrc)
		return -1;

	if (crc > otherCrc)
		return 1;

	char const *string = getString();
	for (int i = 0; ; ++i)
	{
		const int a = string[i];
		const int b = tolower(otherString[i]);

		if (!a || !b || a != b)
		{
			if (a < b)
				return -1;

			if (a > b)
				return 1;

			if (a == b)
				return 0;
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Compare this CrcLowerString with another.
 * 
 * The return value does not indicate the lexiographic ordering of the
 * strings, because they are first compared using the CRC, and only if
 * the CRC's match are they compared by strcmp().
 * 
 * @param otherString  CrcLowerString to compare against
 * @return See Remarks section
 */

int CrcLowerString::compare(const CrcLowerString &otherString) const
{
	return compare(otherString.getString(), otherString.getCrc());
}

// ----------------------------------------------------------------------
/**
 *
 * Perform an assignment of the CrcLowerString object.
 *
 * NOTE: use of this function during game runtime likely is an indication
 * of a flawed design.  This function was added to support tools where
 * simplicity overrode efficiency concerns.
 *
 * @param rhs  the source CrcLowerString object to copy into the left-hand-side object
 */

CrcLowerString &CrcLowerString::operator =(const CrcLowerString &rhs)
{
	set(rhs.getString(), rhs.getCrc());
	return *this;
}

// ======================================================================
