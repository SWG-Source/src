// ======================================================================
//
// GuildMemberInfo.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _GuildMemberInfo_H_
#define _GuildMemberInfo_H_

// ======================================================================

#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class GuildMemberInfo
{
public:
	GuildMemberInfo();
	GuildMemberInfo(GuildMemberInfo const * gmi);
	~GuildMemberInfo();

	bool operator== (const GuildMemberInfo& rhs) const;
	bool operator!= (const GuildMemberInfo& rhs) const;

	std::string m_name;
	std::string m_professionSkillTemplate;
	int m_level;
	int m_permissions;
	BitArray m_rank;
	std::string m_title;
	NetworkId m_allegiance;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, GuildMemberInfo & target);
	void put(ByteStream & target, const GuildMemberInfo & source);
}

// ======================================================================

#endif // _GuildMemberInfo_H_
