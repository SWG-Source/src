// ======================================================================
//
// CrcString.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CrcString.h"

#include "sharedFoundation/Crc.h"

// ======================================================================
/**
 * Normalize a string.
 *
 * This function will clean up a string.  All alpha characters will be
 * changed to lower case, all backslashes will be converted to forward
 * slashes, all dots following a slash will be removed, and consecutive
 * slashes will be converted into a single slash.
 */

void CrcString::normalize(char *output, const char *input)
{
	bool previousIsSlash = true;
	for ( ; *input; ++input)
	{
		const char c = *input;
		if (c == '\\' || c == '/')
		{
			if (!previousIsSlash)
			{
				// convert all backslashes to forward slashes and disallow multiple slashes in a row
				*(output++) = '/';
				previousIsSlash = true;
			}
		}
		else
			if (c == '.')
			{
				// disallow dots after slashes.  this will also handle multiple dots, and slashes following the dots
				if (!previousIsSlash)
					*(output++) = '.';
			}
			else
			{
				// lowercase all other characters
				*(output++) = static_cast<char>(tolower(c));
				previousIsSlash = false;
			}
	}

	*output = '\0';
}

// ======================================================================

CrcString::CrcString()
: m_crc(Crc::crcNull)
{
}

// ----------------------------------------------------------------------

CrcString::CrcString(uint32 crc)
: m_crc(crc)
{
}

// ----------------------------------------------------------------------

CrcString::~CrcString()
{
}

// ----------------------------------------------------------------------

void CrcString::calculateCrc()
{
	m_crc = Crc::calculate(getString());
}

// ----------------------------------------------------------------------

bool CrcString::isEmpty() const
{
	const char * const str = getString ();
	return !str || str[0] == '\0';
}

// ----------------------------------------------------------------------

void CrcString::set(CrcString const & rhs)
{
	if (this == &rhs)
		return;

	if (rhs.isEmpty())
		clear();
	else
		set(rhs.getString(), rhs.getCrc());
}

// ----------------------------------------------------------------------

bool CrcString::operator ==(CrcString const & rhs) const
{
	return !(*this != rhs);
}

// ----------------------------------------------------------------------

bool CrcString::operator !=(CrcString const & rhs) const
{
	return m_crc != rhs.m_crc || strcmp(getString(), rhs.getString()) != 0;
}

// ----------------------------------------------------------------------

bool CrcString::operator < (CrcString const & rhs) const
{
	if (m_crc < rhs.m_crc)
		return true;
	if (m_crc > rhs.m_crc)
		return false;

	return strcmp(getString(), rhs.getString()) < 0;
}

// ----------------------------------------------------------------------

bool CrcString::operator <=(CrcString const & rhs) const
{
	return !(*this > rhs);
}

// ----------------------------------------------------------------------

bool CrcString::operator > (CrcString const & rhs) const
{
	if (m_crc > rhs.m_crc)
		return true;
	if (m_crc < rhs.m_crc)
		return false;

	return strcmp(getString(), rhs.getString()) > 0;
}

// ----------------------------------------------------------------------

bool CrcString::operator >=(CrcString const & rhs) const
{
	return !(*this < rhs);
}

// ======================================================================
