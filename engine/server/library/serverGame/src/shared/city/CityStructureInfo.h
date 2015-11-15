// ======================================================================
//
// CityStructureInfo.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _CityStructureInfo_H_
#define _CityStructureInfo_H_

// ======================================================================

#include "sharedFoundation/NetworkId.h"

class CityStructureInfo;

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, CityStructureInfo & target);
	void put(ByteStream & target, const CityStructureInfo & source);
}

// ======================================================================

class CityStructureInfo
{
	friend void Archive::get(Archive::ReadIterator & source, CityStructureInfo & target);
	friend void Archive::put(Archive::ByteStream & target, const CityStructureInfo & source);

public:
	bool operator== (const CityStructureInfo& rhs) const;
	bool operator!= (const CityStructureInfo& rhs) const;

	int getStructureType() const;
	bool getStructureValid() const;
	void setStructureType(int structureType);
	void setStructureValid(bool structureValid);
private:
	int m_structureType;
	bool m_structureValid;
};

// ----------------------------------------------------------------------

inline int CityStructureInfo::getStructureType() const
{
	return m_structureType;
}

// ----------------------------------------------------------------------

inline bool CityStructureInfo::getStructureValid() const
{
	return m_structureValid;
}

// ----------------------------------------------------------------------

inline void CityStructureInfo::setStructureType(int structureType)
{
	m_structureType = structureType;
}

// ----------------------------------------------------------------------

inline void CityStructureInfo::setStructureValid(bool structureValid)
{
	m_structureValid = structureValid;
}

// ======================================================================

#endif // _CityStructureInfo_H_
