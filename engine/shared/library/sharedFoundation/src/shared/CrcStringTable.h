// ======================================================================
//
// CrcStringTable.h
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CrcStringTable_H
#define INCLUDED_CrcStringTable_H

// ======================================================================

class ConstCharCrcString;
class Iff;

// ======================================================================

class CrcStringTable
{
public:

	CrcStringTable();
	CrcStringTable(const char * fileName);
	~CrcStringTable();

	void load(const char * fileName);
	void load(Iff &iff);

	ConstCharCrcString const lookUp(const char * string) const;
	ConstCharCrcString const lookUp(uint32 crc) const;

	void getAllStrings(std::vector<const char *> & strings) const;

	int getNumberOfStrings() const;
	ConstCharCrcString const getString(int index) const;

private:

	CrcStringTable(CrcStringTable const &);
	CrcStringTable & operator =(CrcStringTable const &);

	void load_0000(Iff &iff);

private:

	int m_numberOfEntries;
	uint32 * m_crcTable;
	int32 * m_stringsOffsetTable;
	char * m_strings;
};

// ======================================================================

#endif
