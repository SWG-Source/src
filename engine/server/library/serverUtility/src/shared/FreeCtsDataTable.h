// ======================================================================
//
// FreeCtsDataTable.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_FreeCtsDataTable_H
#define INCLUDED_FreeCtsDataTable_H

#include <map>
#include <string>

// ======================================================================

class FreeCtsDataTable // static class
{
public:

	struct FreeCtsInfo
	{
		std::string ruleName;
		std::string ruleDescription;
		std::map<std::string, std::string> targetCluster;
		time_t startTime;
		time_t endTime;
		time_t sourceCharacterCreateTimeLower;
		time_t sourceCharacterCreateTimeUpper;
	};

	static std::string const & getFreeCtsFileName();
	static std::multimap<std::string, FreeCtsDataTable::FreeCtsInfo> const & getFreeCtsInfo();
	static FreeCtsDataTable::FreeCtsInfo const * isFreeCtsSourceCluster(std::string const & sourceCluster);
	static FreeCtsDataTable::FreeCtsInfo const * wouldCharacterTransferBeFree(time_t sourceCharacterCreateTime, uint32 sourceStationId, std::string const & sourceCluster, uint32 targetStationId, std::string const & targetCluster, bool ignoreTimeRestriction);
	static FreeCtsDataTable::FreeCtsInfo const * getFreeCtsInfoForCharacter(time_t sourceCharacterCreateTime, std::string const & sourceCluster, bool ignoreTimeRestriction);

private: // disabled

	FreeCtsDataTable();
	FreeCtsDataTable(FreeCtsDataTable const &);
	FreeCtsDataTable &operator =(FreeCtsDataTable const &);
};

// ======================================================================

#endif // INCLUDED_FreeCtsDataTable_H
