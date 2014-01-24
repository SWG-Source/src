// ======================================================================
//
// GuildInfo.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/GuildInfo.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"

#include <algorithm>

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, GuildInfo & target)
	{
		get(source, target.m_name);
		get(source, target.m_abbrev);
		get(source, target.m_leaderId);
		get(source, target.m_guildElectionPreviousEndTime);
		get(source, target.m_guildElectionNextEndTime);
		get(source, target.m_guildFaction);
		get(source, target.m_timeLeftGuildFaction);
		get(source, target.m_guildGcwDefenderRegion);
		get(source, target.m_timeJoinedGuildGcwDefenderRegion);
		get(source, target.m_timeLeftGuildGcwDefenderRegion);
		get(source, target.m_sponsoredCount);
		get(source, target.m_sponsoredAndMemberCount);
		get(source, target.m_guildWarInclusionCount);
		get(source, target.m_guildWarExclusionCount);
	}

	void put(ByteStream & target, const GuildInfo & source)
	{
		put(target, source.m_name);
		put(target, source.m_abbrev);
		put(target, source.m_leaderId);
		put(target, source.m_guildElectionPreviousEndTime);
		put(target, source.m_guildElectionNextEndTime);
		put(target, source.m_guildFaction);
		put(target, source.m_timeLeftGuildFaction);
		put(target, source.m_guildGcwDefenderRegion);
		put(target, source.m_timeJoinedGuildGcwDefenderRegion);
		put(target, source.m_timeLeftGuildGcwDefenderRegion);
		put(target, source.m_sponsoredCount);
		put(target, source.m_sponsoredAndMemberCount);
		put(target, source.m_guildWarInclusionCount);
		put(target, source.m_guildWarExclusionCount);
	}
}

// ======================================================================

GuildInfo::GuildInfo() :
	m_name(),
	m_abbrev(),
	m_leaderId(),
	m_guildElectionPreviousEndTime(0),
	m_guildElectionNextEndTime(0),
	m_guildFaction(0),
	m_timeLeftGuildFaction(0),
	m_guildGcwDefenderRegion(),
	m_timeJoinedGuildGcwDefenderRegion(0),
	m_timeLeftGuildGcwDefenderRegion(0),
	m_sponsoredCount(0),
	m_sponsoredAndMemberCount(0),
	m_guildWarInclusionCount(0),
	m_guildWarExclusionCount(0)
{
}

// ======================================================================

GuildInfo::GuildInfo(GuildInfo const * const gi) :
	m_name(),
	m_abbrev(),
	m_leaderId(),
	m_guildElectionPreviousEndTime(0),
	m_guildElectionNextEndTime(0),
	m_guildFaction(0),
	m_timeLeftGuildFaction(0),
	m_guildGcwDefenderRegion(),
	m_timeJoinedGuildGcwDefenderRegion(0),
	m_timeLeftGuildGcwDefenderRegion(0),
	m_sponsoredCount(0),
	m_sponsoredAndMemberCount(0),
	m_guildWarInclusionCount(0),
	m_guildWarExclusionCount(0)
{
	if (gi)
	{
		m_name = gi->m_name;
		m_abbrev = gi->m_abbrev;
		m_leaderId = gi->m_leaderId;
		m_guildElectionPreviousEndTime = gi->m_guildElectionPreviousEndTime;
		m_guildElectionNextEndTime = gi->m_guildElectionNextEndTime;
		m_guildFaction = gi->m_guildFaction;
		m_timeLeftGuildFaction = gi->m_timeLeftGuildFaction;
		m_guildGcwDefenderRegion = gi->m_guildGcwDefenderRegion;
		m_timeJoinedGuildGcwDefenderRegion = gi->m_timeJoinedGuildGcwDefenderRegion;
		m_timeLeftGuildGcwDefenderRegion = gi->m_timeLeftGuildGcwDefenderRegion;
		m_sponsoredCount = gi->m_sponsoredCount;
		m_sponsoredAndMemberCount = gi->m_sponsoredAndMemberCount;
		m_guildWarInclusionCount = gi->m_guildWarInclusionCount;
		m_guildWarExclusionCount = gi->m_guildWarExclusionCount;
	}
}

// ----------------------------------------------------------------------

GuildInfo::~GuildInfo()
{
}

// ----------------------------------------------------------------------

bool GuildInfo::operator== (const GuildInfo& rhs) const
{
	if (this == &rhs)
		return true;

	return ((m_name == rhs.m_name) &&
		(m_abbrev == rhs.m_abbrev) &&
		(m_leaderId == rhs.m_leaderId) &&
		(m_guildElectionPreviousEndTime == rhs.m_guildElectionPreviousEndTime) &&
		(m_guildElectionNextEndTime == rhs.m_guildElectionNextEndTime) &&
		(m_guildFaction == rhs.m_guildFaction) &&
		(m_timeLeftGuildFaction == rhs.m_timeLeftGuildFaction) &&
		(m_guildGcwDefenderRegion == rhs.m_guildGcwDefenderRegion) &&
		(m_timeJoinedGuildGcwDefenderRegion == rhs.m_timeJoinedGuildGcwDefenderRegion) &&
		(m_timeLeftGuildGcwDefenderRegion == rhs.m_timeLeftGuildGcwDefenderRegion) &&
		(m_sponsoredCount == rhs.m_sponsoredCount) &&
		(m_sponsoredAndMemberCount == rhs.m_sponsoredAndMemberCount) &&
		(m_guildWarInclusionCount == rhs.m_guildWarInclusionCount) &&
		(m_guildWarExclusionCount == rhs.m_guildWarExclusionCount));
}

// ----------------------------------------------------------------------

bool GuildInfo::operator!= (const GuildInfo& rhs) const
{
	return !operator==(rhs);
}

// ----------------------------------------------------------------------

int GuildInfo::getCountMembersOnly() const
{
	return std::max(0, (m_sponsoredAndMemberCount - m_sponsoredCount));
}

// ----------------------------------------------------------------------

int GuildInfo::getCountSponsoredOnly() const
{
	return m_sponsoredCount;
}

// ----------------------------------------------------------------------

int GuildInfo::getCountMembersAndSponsored() const
{
	return m_sponsoredAndMemberCount;
}

// ----------------------------------------------------------------------

int GuildInfo::getCountMembersGuildWarPvPEnabled() const
{
	if (m_guildWarInclusionCount)
		return m_guildWarInclusionCount;

	return std::max(0, (m_sponsoredAndMemberCount - m_sponsoredCount - m_guildWarExclusionCount));
}

// ======================================================================
