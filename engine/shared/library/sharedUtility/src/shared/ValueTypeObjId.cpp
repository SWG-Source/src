// ======================================================================
//
// ValueTypeObjId.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueTypeObjId.h"

#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

std::string const ValueTypeObjId::ms_type = "object id";

// ======================================================================

ValueTypeObjId::ValueTypeObjId(NetworkId const & value) :
	ValueTypeBase(ms_type),
	m_value(value)
{
}

// ----------------------------------------------------------------------

ValueTypeObjId::ValueTypeObjId(ValueTypeObjId const & value) :
	ValueTypeBase(ms_type),
	m_value(value.m_value)
{
}

// ----------------------------------------------------------------------

ValueTypeObjId::ValueTypeObjId(Archive::ReadIterator & source) :
	ValueTypeBase(ms_type),
	m_value()
{
	Archive::get(source, m_value);
}

// ----------------------------------------------------------------------

ValueTypeObjId::~ValueTypeObjId()
{
}

// ----------------------------------------------------------------------

void ValueTypeObjId::pack(Archive::ByteStream & target) const
{
	Archive::put(target, m_value);
}

// ----------------------------------------------------------------------

ValueTypeBase * ValueTypeObjId::clone() const
{
	return new ValueTypeObjId(*this);
}

// ----------------------------------------------------------------------

std::string ValueTypeObjId::getValueAsString() const
{
	return m_value.getValueString();
}

// ----------------------------------------------------------------------

ValueTypeObjId * ValueTypeObjId::unpack(Archive::ReadIterator & source)
{
	return new ValueTypeObjId(source);
}

//--------------------------------------------------------------------

ValueTypeObjId & ValueTypeObjId::operator =(ValueTypeObjId const & rhs)
{
	if (this != &rhs)
	{
		m_value = rhs.m_value;
	}

	return *this;
}

// ======================================================================
