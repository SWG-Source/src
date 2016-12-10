// ======================================================================
//
// LfgCharacterData.h
// Copyright 2008 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_LfgCharacterData_H
#define INCLUDED_LfgCharacterData_H

#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedGame/SharedCreatureObjectTemplate.h"

// ======================================================================

class LfgCharacterData
{
public:
	enum Profession
	{
		Prof_Unknown = 0, // Prof_Unknown must be first and start at 0, and subsequent ones increment by 1
		Prof_Bh,
		Prof_Commando,
		Prof_Entertainer,
		Prof_Jedi,
		Prof_Medic,
		Prof_Officer,
		Prof_Smuggler,
		Prof_Spy,
		Prof_Trader_Domestics,
		Prof_Trader_Engineering,
		Prof_Trader_Munitions,
		Prof_Trader_Structures,
		Prof_NPC,
		Profession_MAX // must be last
	};

	enum Pilot
	{
		Pilot_None,
		Pilot_Neutral,
		Pilot_Imperial,
		Pilot_Rebel,
	};

	LfgCharacterData();

	bool operator== (const LfgCharacterData& rhs) const;
	bool operator!= (const LfgCharacterData& rhs) const;

	static LfgCharacterData::Profession convertSkillTemplateToProfession(const std::string & skillTemplate);
	static std::string getProfessionDebugString(LfgCharacterData::Profession profession);
	static Unicode::String const & getProfessionDisplayString(LfgCharacterData::Profession profession);
	static std::map<std::string, int> const & calculateStatistics(std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData);

	std::string getDebugString() const;

	NetworkId characterId;
	int32 characterCreateTime;
	int characterAge;
	Unicode::String characterName;
	std::string characterLowercaseFirstName;
	SharedCreatureObjectTemplate::Species species;
	SharedCreatureObjectTemplate::Gender gender;
	LfgCharacterData::Profession profession;
	LfgCharacterData::Pilot pilot;
	int16 level;
	uint32 faction;
	std::string guildName;
	std::string guildAbbrev;
	std::string citizenOfCity;
	NetworkId groupId;
	std::string locationPlanet;
	std::string locationRegion;
	std::string locationFactionalPresenceGcwRegion;
	int locationFactionalPresenceGridX; // only meaningful if locationFactionalPresenceGcwRegion is not empty and locationPlanet is a ground zone
	int locationFactionalPresenceGridZ; // only meaningful if locationFactionalPresenceGcwRegion is not empty and locationPlanet is a ground zone
	std::string locationPlayerCity;
	std::set<std::string> ctsSourceGalaxy;
	bool searchableByCtsSourceGalaxy;
	bool displayLocationInSearchResults;
	bool anonymous;
	bool active;
	BitArray characterInterests;
	uint32 sessionActivity;
};

// ======================================================================

// only return a subset of the LfgCharacterData in search results
class LfgCharacterSearchResultData
{
public:
	LfgCharacterSearchResultData();
	LfgCharacterSearchResultData(const LfgCharacterData & lfgData);

	NetworkId characterId;
	Unicode::String characterName;
	SharedCreatureObjectTemplate::Species species;
	LfgCharacterData::Profession profession;
	int16 level;
	uint32 faction;
	std::string guildName;
	std::string guildAbbrev;
	NetworkId groupId;
	std::string locationPlanet;
	std::string locationRegion;
	std::string locationPlayerCity;
	std::set<std::string> ctsSourceGalaxy;
	BitArray characterInterests;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, LfgCharacterData & target);
	void put(ByteStream & target, const LfgCharacterData & source);

	void get(ReadIterator & source, LfgCharacterSearchResultData & target);
	void put(ByteStream & target, const LfgCharacterSearchResultData & source);
}

// ======================================================================

#endif // INCLUDED_LfgCharacterData_H
