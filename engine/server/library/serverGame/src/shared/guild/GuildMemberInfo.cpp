// ======================================================================
//
// GuildMemberInfo.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GuildMemberInfo.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, GuildMemberInfo & target)
	{
		get(source, target.m_name);
		get(source, target.m_professionSkillTemplate);
		get(source, target.m_level);
		get(source, target.m_permissions);
		get(source, target.m_rank);
		get(source, target.m_title);
		get(source, target.m_allegiance);
	}

	void put(ByteStream & target, const GuildMemberInfo & source)
	{
		put(target, source.m_name);
		put(target, source.m_professionSkillTemplate);
		put(target, source.m_level);
		put(target, source.m_permissions);
		put(target, source.m_rank);
		put(target, source.m_title);
		put(target, source.m_allegiance);
	}
}

// ======================================================================

GuildMemberInfo::GuildMemberInfo() :
	m_name(),
	m_professionSkillTemplate(),
	m_level(0),
	m_permissions(0),
	m_rank(),
	m_title(),
	m_allegiance()
{
}

// ----------------------------------------------------------------------

GuildMemberInfo::GuildMemberInfo(GuildMemberInfo const * const gmi) :
	m_name(),
	m_professionSkillTemplate(),
	m_level(0),
	m_permissions(0),
	m_rank(),
	m_title(),
	m_allegiance()
{
	if (gmi)
	{
		m_name = gmi->m_name;
		m_professionSkillTemplate = gmi->m_professionSkillTemplate;
		m_level = gmi->m_level;
		m_permissions = gmi->m_permissions;
		m_rank = gmi->m_rank;
		m_title = gmi->m_title;
		m_allegiance = gmi->m_allegiance;
	}
}

// ----------------------------------------------------------------------

GuildMemberInfo::~GuildMemberInfo()
{
}

// ----------------------------------------------------------------------

bool GuildMemberInfo::operator== (const GuildMemberInfo& rhs) const
{
	if (this == &rhs)
		return true;

	return ((m_name == rhs.m_name) &&
		(m_professionSkillTemplate == rhs.m_professionSkillTemplate) &&
		(m_level == rhs.m_level) &&
		(m_permissions == rhs.m_permissions) &&
		(m_rank == rhs.m_rank) &&
		(m_title == rhs.m_title) &&
		(m_allegiance == rhs.m_allegiance));
}

// ----------------------------------------------------------------------

bool GuildMemberInfo::operator!= (const GuildMemberInfo& rhs) const
{
	return !operator==(rhs);
}

// ======================================================================
