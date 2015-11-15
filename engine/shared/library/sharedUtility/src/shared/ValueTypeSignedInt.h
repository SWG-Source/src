// ======================================================================
//
// ValueTypeSignedInt.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueTypeSignedInt_H
#define INCLUDED_ValueTypeSignedInt_H

// ======================================================================

#include "sharedUtility/ValueTypeBase.h"

// ======================================================================

namespace Archive
{
	class ReadIterator;
}

// ======================================================================

class ValueTypeSignedInt : public ValueTypeBase
{
public:

	explicit ValueTypeSignedInt(signed int value);
	         ValueTypeSignedInt(ValueTypeSignedInt const & value);

	virtual ~ValueTypeSignedInt();

public:

	// Archiving method
	virtual void pack(Archive::ByteStream & target) const;

	// Copy method
	virtual ValueTypeBase *clone() const;

	// Operator
	ValueTypeSignedInt &operator =(ValueTypeSignedInt const & rhs);

	// Operation
	signed int getValue() const;
	virtual std::string getValueAsString() const;
	static ValueTypeSignedInt *unpack(Archive::ReadIterator & source);

private:

	// Only accessible through unpack()
	ValueTypeSignedInt(Archive::ReadIterator & source);

	// Disabled.
	ValueTypeSignedInt();

public:

	static std::string const ms_type;

private:

	signed int m_value;
};

//--------------------------------------------------------------------

inline signed int ValueTypeSignedInt::getValue() const
{
	return m_value;
}

// ======================================================================

#endif
