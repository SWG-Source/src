// ======================================================================
//
// CitizenInfo.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CitizenInfo.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, CitizenInfo & target)
	{
		get(source, target.m_citizenName);
		get(source, target.m_citizenProfessionSkillTemplate);
		get(source, target.m_citizenLevel);
		get(source, target.m_citizenPermissions);
		get(source, target.m_citizenRank);
		get(source, target.m_citizenTitle);
		get(source, target.m_citizenAllegiance);
	}

	void put(ByteStream & target, const CitizenInfo & source)
	{
		put(target, source.m_citizenName);
		put(target, source.m_citizenProfessionSkillTemplate);
		put(target, source.m_citizenLevel);
		put(target, source.m_citizenPermissions);
		put(target, source.m_citizenRank);
		put(target, source.m_citizenTitle);
		put(target, source.m_citizenAllegiance);
	}
}

// ======================================================================

bool CitizenInfo::operator== (const CitizenInfo& rhs) const
{
	if (this == &rhs)
		return true;

	return ((m_citizenName == rhs.m_citizenName) &&
		(m_citizenProfessionSkillTemplate == rhs.m_citizenProfessionSkillTemplate) &&
		(m_citizenLevel == rhs.m_citizenLevel) &&
		(m_citizenPermissions == rhs.m_citizenPermissions) &&
		(m_citizenRank == rhs.m_citizenRank) &&
		(m_citizenTitle == rhs.m_citizenTitle) &&
		(m_citizenAllegiance == rhs.m_citizenAllegiance));
}

// ----------------------------------------------------------------------

bool CitizenInfo::operator!= (const CitizenInfo& rhs) const
{
	return !operator==(rhs);
}

// ======================================================================
