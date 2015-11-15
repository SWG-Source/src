// ======================================================================
//
// ValueTypeBool.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueTypeBool.h"

#include "Archive/Archive.h"

// ======================================================================

std::string const ValueTypeBool::ms_type = "bool";

// ======================================================================

ValueTypeBool::ValueTypeBool(bool const value) :
	ValueTypeBase(ms_type),
	m_value(value)
{
}

// ----------------------------------------------------------------------

ValueTypeBool::ValueTypeBool(ValueTypeBool const & value) :
	ValueTypeBase(ms_type),
	m_value(value.m_value)
{
}

// ----------------------------------------------------------------------

ValueTypeBool::ValueTypeBool(Archive::ReadIterator & source) :
	ValueTypeBase(ms_type),
	m_value(false)
{
	Archive::get(source, m_value);
}

// ----------------------------------------------------------------------

ValueTypeBool::~ValueTypeBool()
{
}

// ----------------------------------------------------------------------

void ValueTypeBool::pack(Archive::ByteStream & target) const
{
	Archive::put(target, m_value);
}

// ----------------------------------------------------------------------

ValueTypeBase * ValueTypeBool::clone() const
{
	return new ValueTypeBool(*this);
}

// ----------------------------------------------------------------------

std::string ValueTypeBool::getValueAsString() const
{
	return std::string(m_value ? "true" : "false");
}

// ----------------------------------------------------------------------

ValueTypeBool * ValueTypeBool::unpack(Archive::ReadIterator & source)
{
	return new ValueTypeBool(source);
}

//--------------------------------------------------------------------

ValueTypeBool & ValueTypeBool::operator =(ValueTypeBool const & rhs)
{
	if (this != &rhs)
	{
		m_value = rhs.m_value;
	}

	return *this;
}

// ======================================================================
