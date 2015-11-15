// ======================================================================
//
// ValueTypeBase.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueTypeBase_H
#define INCLUDED_ValueTypeBase_H

// ======================================================================

#include <string>

// ======================================================================

namespace Archive
{
	class ByteStream;
}

// ======================================================================
// this is an interface class intended to be used to encapsulate
// different data types; derive a class for different data types
// ======================================================================

class ValueTypeBase
{
public:

	ValueTypeBase(std::string const & type);
	virtual ~ValueTypeBase();

public:

	// Archiving method
	virtual void pack(Archive::ByteStream & target) const = 0;

	// Copy method
	virtual ValueTypeBase *clone() const = 0;

	// Operation
	virtual std::string getValueAsString() const = 0;
	std::string const & getType() const;

private:
	std::string const m_type;

	// Disabled.
	ValueTypeBase();
	ValueTypeBase(ValueTypeBase const &);
	ValueTypeBase &operator =(ValueTypeBase const &);

};

//--------------------------------------------------------------------

inline std::string const & ValueTypeBase::getType() const
{
	return m_type;
}

// ======================================================================

#endif
