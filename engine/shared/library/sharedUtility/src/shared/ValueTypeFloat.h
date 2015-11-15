// ======================================================================
//
// ValueTypeFloat.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueTypeFloat_H
#define INCLUDED_ValueTypeFloat_H

// ======================================================================

#include "sharedUtility/ValueTypeBase.h"

// ======================================================================

namespace Archive
{
	class ReadIterator;
}

// ======================================================================

class ValueTypeFloat : public ValueTypeBase
{
public:

	explicit ValueTypeFloat(float value);
	         ValueTypeFloat(ValueTypeFloat const & value);

	virtual ~ValueTypeFloat();

public:

	// Archiving method
	virtual void pack(Archive::ByteStream & target) const;

	// Copy method
	virtual ValueTypeBase *clone() const;

	// Operator
	ValueTypeFloat &operator =(ValueTypeFloat const & rhs);

	// Operation
	float getValue() const;
	virtual std::string getValueAsString() const;
	static ValueTypeFloat *unpack(Archive::ReadIterator & source);

private:

	// Only accessible through unpack()
	ValueTypeFloat(Archive::ReadIterator & source);

	// Disabled.
	ValueTypeFloat();

public:

	static std::string const ms_type;

private:

	float m_value;
};

//--------------------------------------------------------------------

inline float ValueTypeFloat::getValue() const
{
	return m_value;
}

// ======================================================================

#endif
