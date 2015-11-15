// ======================================================================
//
// TemporaryCrcString.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TemporaryCrcString_H
#define INCLUDED_TemporaryCrcString_H

// ======================================================================

#include "sharedFoundation/CrcString.h"

// ======================================================================

class TemporaryCrcString : public CrcString
{
public:

	TemporaryCrcString();
	TemporaryCrcString(const TemporaryCrcString &);
	DLLEXPORT TemporaryCrcString(const char *string, bool applyNormalize);
	TemporaryCrcString(const char *string, uint32 crc);

	virtual DLLEXPORT ~TemporaryCrcString();
	
	virtual const char *getString() const;

	virtual void clear();
	virtual void set(const char *string, bool applyNormalize);
	virtual void set(const char *string, uint32 crc);

private:

	TemporaryCrcString & operator =(const TemporaryCrcString &);

	void internalSet(const char *string, bool applyNormalize);

private:

	enum
	{
		BUFFER_SIZE = 512
	};

private:

	char   m_buffer[BUFFER_SIZE];
};

// ======================================================================

#endif
