// ======================================================================
//
// TemporaryCrcString.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/TemporaryCrcString.h"

#include "sharedFoundation/Os.h"
#include "sharedFoundation/Crc.h"

// ======================================================================

TemporaryCrcString::TemporaryCrcString()
: CrcString()
{
	// this is to avoid including Os in the header file
	DEBUG_FATAL(static_cast<int>(BUFFER_SIZE) != static_cast<int>(Os::MAX_PATH_LENGTH), ("Os::MAX_PATH_LENGTH and BUFFER_SIZE differ"));

	m_buffer[0] = '\0';
}

// ----------------------------------------------------------------------
/**
 * Copy constructor.
 *
 * TRF needed this to allow useful sanity checking with std::set<TemporaryCrcString>.
 */
TemporaryCrcString::TemporaryCrcString(const TemporaryCrcString &rhs)
: CrcString()
{
	set(rhs.getString(), rhs.getCrc());
}

// ----------------------------------------------------------------------

TemporaryCrcString::TemporaryCrcString(char const * string, bool applyNormalize)
: CrcString()
{
	// this is to avoid including Os in the header file
	DEBUG_FATAL(static_cast<int>(BUFFER_SIZE) != static_cast<int>(Os::MAX_PATH_LENGTH), ("Os::MAX_PATH_LENGTH and BUFFER_SIZE differ"));

	set(string, applyNormalize);
}

// ----------------------------------------------------------------------

TemporaryCrcString::TemporaryCrcString(char const * string, uint32 crc)
: CrcString()
{
	// this is to avoid including Os in the header file
	DEBUG_FATAL(static_cast<int>(BUFFER_SIZE) != static_cast<int>(Os::MAX_PATH_LENGTH), ("Os::MAX_PATH_LENGTH and BUFFER_SIZE differ"));

	set(string, crc);
}

// ----------------------------------------------------------------------

TemporaryCrcString::~TemporaryCrcString()
{
}

// ----------------------------------------------------------------------

char const * TemporaryCrcString::getString() const
{
	return m_buffer;
}

// ----------------------------------------------------------------------

void TemporaryCrcString::clear()
{
	m_buffer[0] = '\0';
	m_crc = Crc::crcNull;
}

// ----------------------------------------------------------------------

void TemporaryCrcString::internalSet(char const * string, bool applyNormalize)
{
	DEBUG_FATAL(strlen(string)+1 > BUFFER_SIZE, ("string too long %d/%d", strlen(string)+1, BUFFER_SIZE));
	if (applyNormalize)
		normalize(m_buffer, string);
	else
		strcpy(m_buffer, string);
}

// ----------------------------------------------------------------------

void TemporaryCrcString::set(char const * string, bool applyNormalize)
{
	NOT_NULL(string);
	internalSet(string, applyNormalize);
	calculateCrc();
}

// ----------------------------------------------------------------------

void TemporaryCrcString::set(char const * string, uint32 crc)
{
	NOT_NULL(string);
	internalSet(string, false);
	m_crc = crc;
}

// ======================================================================
