// ======================================================================
//
// ValueTypeString.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueTypeString.h"

#include "Archive/Archive.h"

// ======================================================================

std::string const ValueTypeString::ms_type = "string";

// ======================================================================

ValueTypeString::ValueTypeString(std::string const & value) :
	ValueTypeBase(ms_type),
	m_value(value)
{
}

// ----------------------------------------------------------------------

ValueTypeString::ValueTypeString(ValueTypeString const & value) :
	ValueTypeBase(ms_type),
	m_value(value.m_value)
{
}

// ----------------------------------------------------------------------

ValueTypeString::ValueTypeString(Archive::ReadIterator & source) :
	ValueTypeBase(ms_type),
	m_value()
{
	Archive::get(source, m_value);
}

// ----------------------------------------------------------------------

ValueTypeString::~ValueTypeString()
{
}

// ----------------------------------------------------------------------

void ValueTypeString::pack(Archive::ByteStream & target) const
{
	Archive::put(target, m_value);
}

// ----------------------------------------------------------------------

ValueTypeBase * ValueTypeString::clone() const
{
	return new ValueTypeString(*this);
}

// ----------------------------------------------------------------------

std::string ValueTypeString::getValueAsString() const
{
	return m_value;
}

// ----------------------------------------------------------------------

ValueTypeString * ValueTypeString::unpack(Archive::ReadIterator & source)
{
	return new ValueTypeString(source);
}

//--------------------------------------------------------------------

ValueTypeString & ValueTypeString::operator =(ValueTypeString const & rhs)
{
	if (this != &rhs)
	{
		m_value = rhs.m_value;
	}

	return *this;
}

// ======================================================================
