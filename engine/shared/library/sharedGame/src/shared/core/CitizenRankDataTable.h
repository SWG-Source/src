// ======================================================================
//
// CitizenRankDataTable.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_CitizenRankDataTable_H
#define INCLUDED_CitizenRankDataTable_H

#include <vector>

// ======================================================================

class CitizenRankDataTable // static class
{
public:
	static void install();
	static void remove();

	class CitizenRank
	{
	public:
		CitizenRank(std::string const & pName, int pSlotId, std::vector<std::string> const & pTitles) :
			name(pName), slotId(pSlotId), titles(pTitles) {};

		std::string const name;
		int const slotId;
		std::vector<std::string> const titles;

	private:
		CitizenRank();
		CitizenRank(CitizenRank const &);
		CitizenRank &operator =(CitizenRank const &);	
	};

	static std::vector<std::string> const & getAllRankNames();
	static std::vector<CitizenRankDataTable::CitizenRank const *> const & getAllRanks();
	static std::vector<CitizenRankDataTable::CitizenRank const *> const & getAllTitleableRanks();
	static CitizenRankDataTable::CitizenRank const * getRank(std::string const & rankName);
	static CitizenRankDataTable::CitizenRank const * isARankTitle(std::string const & rankTitle);

private: // disabled

	CitizenRankDataTable();
	CitizenRankDataTable(CitizenRankDataTable const &);
	CitizenRankDataTable &operator =(CitizenRankDataTable const &);
};

// ======================================================================

#endif // INCLUDED_CitizenRankDataTable_H
