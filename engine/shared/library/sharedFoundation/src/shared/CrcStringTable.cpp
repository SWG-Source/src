// ======================================================================
//
// CrcStringTable.cpp
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CrcStringTable.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ConstCharCrcString.h"

#include <vector>

// ======================================================================

namespace CrcStringTableNamespace
{
	const Tag TAG_CSTB = TAG(C,S,T,B);
	const Tag TAG_CRCT = TAG(C,R,C,T);
	const Tag TAG_STNG = TAG(S,T,N,G);
	const Tag TAG_STRT = TAG(S,T,R,T);
}
using namespace CrcStringTableNamespace;

// ======================================================================

CrcStringTable::CrcStringTable()
:
	m_numberOfEntries(0),
	m_crcTable(nullptr),
	m_stringsOffsetTable(nullptr),
	m_strings(nullptr)
{
}

// ----------------------------------------------------------------------

CrcStringTable::CrcStringTable(char const * fileName)
:
	m_numberOfEntries(0),
	m_crcTable(nullptr),
	m_stringsOffsetTable(nullptr),
	m_strings(nullptr)
{
	load(fileName);
}

// ----------------------------------------------------------------------

CrcStringTable::~CrcStringTable()
{
	delete [] m_crcTable;
	delete [] m_stringsOffsetTable;
	delete [] m_strings;
}

//----------------------------------------------------------------------

void CrcStringTable::load(char const * fileName)
{
	Iff iff;
	bool result = iff.open(fileName, true);
	if(!result)
	{
		if(fileName)
			DEBUG_WARNING(true, ("Could not load CrcStringTable %s", fileName));
		else
			DEBUG_WARNING(true, ("Could not load CrcStringTable, nullptr file given"));
		return;
	}

	load(iff);

#ifdef _DEBUG
	for (int i = 0; i < m_numberOfEntries; ++i)
	{
		const char *string = m_strings + m_stringsOffsetTable[i];
		uint32 crc = Crc::calculate(string);
		DEBUG_FATAL(m_crcTable[i] != crc, ("Crcs do not match for %s: %08x %08x", string, m_crcTable[i], crc));
	}
#endif
}

// ----------------------------------------------------------------------

void CrcStringTable::load(Iff &iff)
{
	iff.enterForm(TAG_CSTB);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown crc string table version in %s", buffer));
				}
				break;
		}

	iff.exitForm(TAG_CSTB);
}

// ----------------------------------------------------------------------

void CrcStringTable::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			m_numberOfEntries = iff.read_int32();
		iff.exitChunk(TAG_DATA);

		iff.enterChunk(TAG_CRCT);
			m_crcTable = iff.read_uint32(m_numberOfEntries);
		iff.exitChunk(TAG_CRCT);

		iff.enterChunk(TAG_STRT);
			m_stringsOffsetTable = iff.read_int32(m_numberOfEntries);
		iff.exitChunk(TAG_STRT);

		iff.enterChunk(TAG_STNG);
			m_strings = iff.readRest_char();
		iff.exitChunk(TAG_STNG);

		for (int i = 0; i < m_numberOfEntries; ++i)
			m_crcTable[i] = Crc::calculate(m_strings + m_stringsOffsetTable[i]);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

ConstCharCrcString const CrcStringTable::lookUp(const char * string) const
{
	return lookUp(Crc::calculate(string));
}

// ----------------------------------------------------------------------

ConstCharCrcString const CrcStringTable::lookUp(uint32 crc) const
{
	int low = 0;
	int high = m_numberOfEntries - 1;

	while (low <= high)
	{
		int const mid = (low + high) / 2;
		uint32 const entry = m_crcTable[mid];
		if (crc == entry)
			return ConstCharCrcString(m_strings + m_stringsOffsetTable[mid], crc);
		else
			if (crc > entry)
				low = mid + 1;
			else
				high = mid - 1;
	}

	return ConstCharCrcString("", 0);
}

// ----------------------------------------------------------------------

void CrcStringTable::getAllStrings(std::vector<const char *> & strings) const
{
	strings.reserve(strings.size() + m_numberOfEntries);

	for (int i = 0; i < m_numberOfEntries; ++i)
		strings.push_back(m_strings + m_stringsOffsetTable[i]);
}

// ----------------------------------------------------------------------

int CrcStringTable::getNumberOfStrings() const
{
	return m_numberOfEntries;
}

// ----------------------------------------------------------------------

ConstCharCrcString const CrcStringTable::getString(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfStrings());
	return ConstCharCrcString(m_strings + m_stringsOffsetTable[index]);
}

// ======================================================================
