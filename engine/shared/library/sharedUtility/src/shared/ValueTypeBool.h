// ======================================================================
//
// ValueTypeBool.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueTypeBool_H
#define INCLUDED_ValueTypeBool_H

// ======================================================================

#include "sharedUtility/ValueTypeBase.h"

// ======================================================================

namespace Archive
{
	class ReadIterator;
}

// ======================================================================

class ValueTypeBool : public ValueTypeBase
{
public:

	explicit ValueTypeBool(bool value);
	         ValueTypeBool(ValueTypeBool const & value);

	virtual ~ValueTypeBool();

public:

	// Archiving method
	virtual void pack(Archive::ByteStream & target) const;

	// Copy method
	virtual ValueTypeBase *clone() const;

	// Operator
	ValueTypeBool &operator =(ValueTypeBool const & rhs);

	// Operation
	bool getValue() const;
	virtual std::string getValueAsString() const;
	static ValueTypeBool *unpack(Archive::ReadIterator & source);

private:

	// Only accessible through unpack()
	ValueTypeBool(Archive::ReadIterator & source);

	// Disabled.
	ValueTypeBool();

public:

	static std::string const ms_type;

private:

	bool m_value;
};

//--------------------------------------------------------------------

inline bool ValueTypeBool::getValue() const
{
	return m_value;
}

// ======================================================================

#endif
