// ======================================================================
//
// ValueTypeFloat.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueTypeFloat.h"

#include "Archive/Archive.h"

// ======================================================================

std::string const ValueTypeFloat::ms_type = "float";

// ======================================================================

ValueTypeFloat::ValueTypeFloat(float const value) :
	ValueTypeBase(ms_type),
	m_value(value)
{
}

// ----------------------------------------------------------------------

ValueTypeFloat::ValueTypeFloat(ValueTypeFloat const & value) :
	ValueTypeBase(ms_type),
	m_value(value.m_value)
{
}

// ----------------------------------------------------------------------

ValueTypeFloat::ValueTypeFloat(Archive::ReadIterator & source) :
	ValueTypeBase(ms_type),
	m_value(0.0f)
{
	Archive::get(source, m_value);
}

// ----------------------------------------------------------------------

ValueTypeFloat::~ValueTypeFloat()
{
}

// ----------------------------------------------------------------------

void ValueTypeFloat::pack(Archive::ByteStream & target) const
{
	Archive::put(target, m_value);
}

// ----------------------------------------------------------------------

ValueTypeBase * ValueTypeFloat::clone() const
{
	return new ValueTypeFloat(*this);
}

// ----------------------------------------------------------------------

std::string ValueTypeFloat::getValueAsString() const
{
	char buffer[128];

	snprintf(buffer, sizeof(buffer)-1, "%.2f", m_value);
	buffer[sizeof(buffer)-1] = '\0';

	return std::string(buffer);
}

// ----------------------------------------------------------------------

ValueTypeFloat * ValueTypeFloat::unpack(Archive::ReadIterator & source)
{
	return new ValueTypeFloat(source);
}

//--------------------------------------------------------------------

ValueTypeFloat & ValueTypeFloat::operator =(ValueTypeFloat const & rhs)
{
	if (this != &rhs)
	{
		m_value = rhs.m_value;
	}

	return *this;
}

// ======================================================================
