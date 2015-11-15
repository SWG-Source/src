// ======================================================================
//
// GuildRankDataTable.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_GuildRankDataTable_H
#define INCLUDED_GuildRankDataTable_H

#include <vector>

// ======================================================================

class GuildRankDataTable // static class
{
public:
	static void install();
	static void remove();

	class GuildRank
	{
	public:
		GuildRank(std::string const & pName, std::string const & pDisplayName, int pSlotId, std::vector<std::string> const & pTitles) :
			name(pName), displayName(pDisplayName), slotId(pSlotId), titles(pTitles) {};

		std::string const name;
		std::string const displayName;
		int const slotId;
		std::vector<std::string> const titles;

	private:
		GuildRank();
		GuildRank(GuildRank const &);
		GuildRank &operator =(GuildRank const &);	
	};

	static std::vector<std::string> const & getAllRankNames();
	static std::vector<GuildRankDataTable::GuildRank const *> const & getAllRanks();
	static std::vector<GuildRankDataTable::GuildRank const *> const & getAllTitleableRanks();
	static GuildRankDataTable::GuildRank const * getRank(std::string const & rankName);
	static GuildRankDataTable::GuildRank const * getRankForDisplayRankName(std::string const & displayRankName);
	static GuildRankDataTable::GuildRank const * isARankTitle(std::string const & rankTitle);

private: // disabled

	GuildRankDataTable();
	GuildRankDataTable(GuildRankDataTable const &);
	GuildRankDataTable &operator =(GuildRankDataTable const &);
};

// ======================================================================

#endif // INCLUDED_GuildRankDataTable_H
