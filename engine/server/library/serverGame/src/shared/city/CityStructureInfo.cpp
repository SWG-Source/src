// ======================================================================
//
// CityStructureInfo.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CityStructureInfo.h"

#include "Archive/Archive.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, CityStructureInfo & target)
	{
		get(source, target.m_structureType);
		get(source, target.m_structureValid);
	}

	void put(ByteStream & target, const CityStructureInfo & source)
	{
		put(target, source.m_structureType);
		put(target, source.m_structureValid);
	}
}

// ======================================================================

bool CityStructureInfo::operator== (const CityStructureInfo& rhs) const
{
	if (this == &rhs)
		return true;

	return ((m_structureType == rhs.m_structureType) &&
		(m_structureValid == rhs.m_structureValid));
}

// ----------------------------------------------------------------------

bool CityStructureInfo::operator!= (const CityStructureInfo& rhs) const
{
	return !operator==(rhs);
}

// ======================================================================
