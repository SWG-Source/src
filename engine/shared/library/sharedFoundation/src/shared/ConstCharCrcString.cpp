// ======================================================================
//
// ConstCharCrcString.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/ConstCharCrcString.h"

// ======================================================================

ConstCharCrcString::ConstCharCrcString(const char *string)
: CrcString(),
	m_buffer(string)
{
	calculateCrc ();
}

// ----------------------------------------------------------------------

ConstCharCrcString::ConstCharCrcString(const char *string, uint32 crc)
: CrcString(),
	m_buffer(string)
{
#ifdef _DEBUG
	calculateCrc();
	DEBUG_FATAL(m_crc != crc, ("calculated and specified crc values do not match"));
#endif
	m_crc = crc;
}

// ----------------------------------------------------------------------

ConstCharCrcString::ConstCharCrcString(ConstCharCrcString const &rhs)
: CrcString(rhs.m_crc),
	m_buffer(rhs.m_buffer)
{
}

// ----------------------------------------------------------------------

ConstCharCrcString::~ConstCharCrcString()
{
}

// ----------------------------------------------------------------------

char const * ConstCharCrcString::getString() const
{
	return m_buffer;
}

// ----------------------------------------------------------------------

void ConstCharCrcString::clear()
{
	DEBUG_FATAL(true, ("cannot clear a ConstCharCrcString"));
}

// ----------------------------------------------------------------------

void ConstCharCrcString::set(char const *, bool)
{
	DEBUG_FATAL(true, ("cannot clear a ConstCharCrcString"));
}

// ----------------------------------------------------------------------

void ConstCharCrcString::set(char const *, uint32)
{
	DEBUG_FATAL(true, ("cannot clear a ConstCharCrcString"));
}

// ======================================================================
