// ======================================================================
//
// PersistentCrcString.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PersistentCrcString_H
#define INCLUDED_PersistentCrcString_H

// ======================================================================

class MemoryBlockManager;
#include "sharedFoundation/CrcString.h"

// ======================================================================

class PersistentCrcString : public CrcString
{
public:

	static PersistentCrcString const empty;

public:

	static void install();

public:

	PersistentCrcString();
	DLLEXPORT explicit PersistentCrcString(CrcString const & rhs);
	PersistentCrcString(PersistentCrcString const & rhs);
	PersistentCrcString(char const * string, bool applyNormalize);
	PersistentCrcString(char const * string, uint32 crc);

	virtual DLLEXPORT ~PersistentCrcString();
	
	virtual char const * getString() const;

	virtual void clear();
	virtual void set(char const * string, bool applyNormalize);
	virtual void set(char const * string, uint32 crc);

	PersistentCrcString const & operator=(PersistentCrcString const & rhs);

protected:

	enum ConstChar
	{
		CC_true
	};

	PersistentCrcString(char const * string, ConstChar);

private:

	void internalFree();
	void internalSet(char const * string, bool applyNormalize);

private:

	MemoryBlockManager *  m_memoryBlockManager;
	char *                m_buffer;
};

// ======================================================================

#endif
