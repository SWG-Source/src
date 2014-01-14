// ======================================================================
//
// ValueTypeString.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueTypeString_H
#define INCLUDED_ValueTypeString_H

// ======================================================================

#include "sharedUtility/ValueTypeBase.h"

#include <string>

// ======================================================================

namespace Archive
{
	class ReadIterator;
}

// ======================================================================

class ValueTypeString : public ValueTypeBase
{
public:

	ValueTypeString(std::string const & value);
	ValueTypeString(ValueTypeString const & value);

	virtual ~ValueTypeString();

public:

	// Archiving method
	virtual void pack(Archive::ByteStream & target) const;

	// Copy method
	virtual ValueTypeBase *clone() const;

	// Operator
	ValueTypeString &operator =(ValueTypeString const & rhs);

	// Operation
	std::string const & getValue() const;
	virtual std::string getValueAsString() const;
	static ValueTypeString *unpack(Archive::ReadIterator & source);

private:

	// Only accessible through unpack()
	ValueTypeString(Archive::ReadIterator & source);

	// Disabled.
	ValueTypeString();

public:

	static std::string const ms_type;

private:

	std::string m_value;
};

//--------------------------------------------------------------------

inline std::string const & ValueTypeString::getValue() const
{
	return m_value;
}

// ======================================================================

#endif
