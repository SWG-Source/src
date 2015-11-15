// ======================================================================
//
// ConstCharCrcString.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ConstCharCrcString_H
#define INCLUDED_ConstCharCrcString_H

// ======================================================================

#include "sharedFoundation/CrcString.h"

// ======================================================================

class ConstCharCrcString : public CrcString
{
public:

	ConstCharCrcString(const char *string);
	ConstCharCrcString(const char *string, uint32 crc);
	ConstCharCrcString(ConstCharCrcString const &);
	virtual ~ConstCharCrcString();

	virtual char const * getString() const;

	virtual void         clear();
	virtual void         set(char const * string, bool applyNormalize);
	virtual void         set(char const * string, uint32 crc);

private:

	ConstCharCrcString();
	ConstCharCrcString & operator =(ConstCharCrcString const &);

private:

	char const * const m_buffer;
};

// ======================================================================

#endif
