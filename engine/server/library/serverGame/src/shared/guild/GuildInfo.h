// ======================================================================
//
// GuildInfo.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _GuildInfo_H_
#define _GuildInfo_H_

// ======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

class GuildInfo
{
public:
	GuildInfo();
	GuildInfo(GuildInfo const * gi);
	~GuildInfo();

	bool operator== (const GuildInfo& rhs) const;
	bool operator!= (const GuildInfo& rhs) const;

	int getCountMembersOnly() const;
	int getCountSponsoredOnly() const;
	int getCountMembersAndSponsored() const;
	int getCountMembersGuildWarPvPEnabled() const;

	std::string m_name;
	std::string m_abbrev;
	NetworkId m_leaderId;
	int m_guildElectionPreviousEndTime;
	int m_guildElectionNextEndTime;
	uint32 m_guildFaction;
	int m_timeLeftGuildFaction;
	std::string m_guildGcwDefenderRegion;
	int m_timeJoinedGuildGcwDefenderRegion;
	int m_timeLeftGuildGcwDefenderRegion;

	int m_sponsoredCount;
	int m_sponsoredAndMemberCount;
	int m_guildWarInclusionCount;
	int m_guildWarExclusionCount;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, GuildInfo & target);
	void put(ByteStream & target, const GuildInfo & source);
}
// ======================================================================

#endif // _GuildInfo_H_
