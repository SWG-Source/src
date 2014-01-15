// ======================================================================
//
// CityStringParser.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _CityStringParser_H_
#define _CityStringParser_H_

// ======================================================================

class BitArray;
class NetworkId;
class PgcRatingInfo;
class Vector;

// ======================================================================

class CityStringParser // static
{
public:
	static bool parseCityInfo(std::string const &source, int &cityId, std::string &cityName, NetworkId &cityHallId, std::string &cityPlanet, int &x, int &z, int &radius, int &creationTime, NetworkId &leaderId, int &incomeTax, int &propertyTax, int &salesTax, Vector &travelLoc, int &travelCost, bool &travelInterplanetary, Vector &cloneLoc, Vector &cloneRespawn, NetworkId &cloneRespawnCell, NetworkId &cloneId);
	static bool parseCitizenInfo(std::string const &source, int &cityId, NetworkId &citizenId, std::string &citizenName, std::string &citizenProfessionSkillTemplate, int &citizenLevel, int &citizenPermissions, BitArray &citizenRank, std::string &citizenTitle, NetworkId &citizenAllegiance);
	static bool parseCityStructureInfo(std::string const &source, int &cityId, NetworkId &structureId, int &structureType, bool &structureValid);
	static bool parsePgcRatingInfo(std::string const &source, NetworkId &chroniclerId, PgcRatingInfo &pgcRatingInfo);

	static void buildCitySpec(int cityId, std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, int creationTime, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId, std::string &result);
	static void buildCitizenSpec(int cityId, NetworkId const &citizenId, std::string const &citizenName, std::string const &citizenProfessionSkillTemplate, int citizenLevel, int citizenPermissions, BitArray const &citizenRank, std::string const &citizenTitle, NetworkId const &citizenAllegiance, std::string &result);
	static void buildCityStructureSpec(int cityId, NetworkId const &structureId, int structureType, bool structureValid, std::string &result);
	static void buildPgcRatingSpec(NetworkId const &chroniclerId, PgcRatingInfo const &pgcRatingInfo, std::string &result);

private:
	CityStringParser();
};

// ======================================================================

#endif // _CityStringParser_H_

