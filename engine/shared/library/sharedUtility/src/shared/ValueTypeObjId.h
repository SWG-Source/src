// ======================================================================
//
// ValueTypeObjId.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ValueTypeObjId_H
#define INCLUDED_ValueTypeObjId_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/ValueTypeBase.h"

// ======================================================================

namespace Archive
{
	class ReadIterator;
}

// ======================================================================

class ValueTypeObjId : public ValueTypeBase
{
public:

	explicit ValueTypeObjId(NetworkId const & value);
	         ValueTypeObjId(ValueTypeObjId const & value);

	virtual ~ValueTypeObjId();

public:

	// Archiving method
	virtual void pack(Archive::ByteStream & target) const;

	// Copy method
	virtual ValueTypeBase *clone() const;

	// Operator
	ValueTypeObjId &operator =(ValueTypeObjId const & rhs);

	// Operation
	NetworkId const & getValue() const;
	virtual std::string getValueAsString() const;
	static ValueTypeObjId *unpack(Archive::ReadIterator & source);

private:

	// Only accessible through unpack()
	ValueTypeObjId(Archive::ReadIterator & source);

	// Disabled.
	ValueTypeObjId();

public:

	static std::string const ms_type;

private:

	NetworkId m_value;
};

//--------------------------------------------------------------------

inline NetworkId const & ValueTypeObjId::getValue() const
{
	return m_value;
}

// ======================================================================

#endif
