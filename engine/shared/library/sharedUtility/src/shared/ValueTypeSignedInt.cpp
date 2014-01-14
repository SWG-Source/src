// ======================================================================
//
// ValueTypeSignedInt.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueTypeSignedInt.h"

#include "Archive/Archive.h"

// ======================================================================

std::string const ValueTypeSignedInt::ms_type = "signed int";

// ======================================================================

ValueTypeSignedInt::ValueTypeSignedInt(signed int const value) :
	ValueTypeBase(ms_type),
	m_value(value)
{
}

// ----------------------------------------------------------------------

ValueTypeSignedInt::ValueTypeSignedInt(ValueTypeSignedInt const & value) :
	ValueTypeBase(ms_type),
	m_value(value.m_value)
{
}

// ----------------------------------------------------------------------

ValueTypeSignedInt::ValueTypeSignedInt(Archive::ReadIterator & source) :
	ValueTypeBase(ms_type),
	m_value(0)
{
	int32 value;
	Archive::get(source, value);
	m_value = static_cast<signed int>(value);
}

// ----------------------------------------------------------------------

ValueTypeSignedInt::~ValueTypeSignedInt()
{
}

// ----------------------------------------------------------------------

void ValueTypeSignedInt::pack(Archive::ByteStream & target) const
{
	Archive::put(target, static_cast<int32>(m_value));
}

// ----------------------------------------------------------------------

ValueTypeBase * ValueTypeSignedInt::clone() const
{
	return new ValueTypeSignedInt(*this);
}

// ----------------------------------------------------------------------

std::string ValueTypeSignedInt::getValueAsString() const
{
	char buffer[32];

	snprintf(buffer, sizeof(buffer)-1, "%d", m_value);
	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

// ----------------------------------------------------------------------

ValueTypeSignedInt * ValueTypeSignedInt::unpack(Archive::ReadIterator & source)
{
	return new ValueTypeSignedInt(source);
}

//--------------------------------------------------------------------

ValueTypeSignedInt & ValueTypeSignedInt::operator =(ValueTypeSignedInt const & rhs)
{
	if (this != &rhs)
	{
		m_value = rhs.m_value;
	}

	return *this;
}

// ======================================================================
