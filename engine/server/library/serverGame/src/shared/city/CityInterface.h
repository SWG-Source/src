// ======================================================================
//
// CityInterface.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _CityInterface_H_
#define _CityInterface_H_

// ======================================================================

class CreatureObject;
class CitizenInfo;
class CityInfo;
class CityObject;
class CityStructureInfo;
class NetworkId;
class PgcRatingInfo;
class Vector;

// ======================================================================

class CityInterface // static
{
public:
	static std::vector<int> const &getCitizenOfCityId(NetworkId const &citizenId);
	static int getMilitiaOfCityId(NetworkId const &citizenId);
	static void getAllCityIds(std::vector<int> &results);
	static int findCityByName(std::string const &name);
	static int findCityByCityHall(NetworkId const &cityHallId);
	static int getCityAtLocation(std::string const &planetName, int x, int z, int radius);
	static bool cityExists(int cityId);
	static void getCitizenIds(int cityId, std::vector<NetworkId> &results);
	static int getCitizenCount(int cityId);
	static void getCityStructureIds(int cityId, std::vector<NetworkId> &results);
	static CityInfo const &getCityInfo(int cityId);
	static std::map<int, CityInfo> const &getAllCityInfo();
	static CitizenInfo const *getCitizenInfo(int cityId, NetworkId const &citizenId);
	static std::map<std::pair<int, NetworkId>, CitizenInfo> const &getAllCitizensInfo();
	static CityStructureInfo const *getCityStructureInfo(int cityId, NetworkId const &structureId);
	static std::map<std::pair<int, NetworkId>, CityStructureInfo> const & getAllCityStructuresInfo();
	static std::map<std::pair<std::string, int>, uint32> const & getGcwRegionDefenderCities();
	static std::map<std::string, std::pair<int, int> > const & getGcwRegionDefenderCitiesCount();
	static int getGcwRegionDefenderCitiesVersion();

	static bool isValidCityName(std::string const &cityName);
	static bool isCityMilitia(int cityId, NetworkId const &citizenId);
	static int createCity(std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId);
	static void removeCity(int cityId);
	static void removeCitizen(int cityId, NetworkId const &citizenId, bool sendUpdateNotification = true);
	static void removeCityStructure(int cityId, NetworkId const &structureId);
	static void setCityName(int cityId, std::string const &cityName);
	static void setCityHall(int cityId, NetworkId const &cityHallId);
	static void setCityLocation(int cityId, std::string const &cityPlanet, int x, int z);
	static void setCityRadius(int cityId, int radius);
	static void setCityFaction(int cityId, uint32 faction, bool notifyCitizens);
	static void setCityGcwDefenderRegion(int cityId, std::string const &gcwDefenderRegion, int timeJoined, bool notifyCitizens);
	static void setCityCreationTime(int cityId, int creationTime);
	static void setCityLeader(int cityId, NetworkId const &leaderId);
	static void setCityIncomeTax(int cityId, int incomeTax);
	static void setCityPropertyTax(int cityId, int propertyTax);
	static void setCitySalesTax(int cityId, int salesTax);
	static void setCityTravelInfo(int cityId, Vector const &travelLoc, int travelCost, bool travelInterplanetary);
	static void setCityCloneInfo(int cityId, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId);
	static void setCitizenInfo(int cityId, NetworkId const &citizenId, std::string const &citizenName, NetworkId const &allegiance, int permissions);
	static void setCitizenProfessionInfo(int cityId, NetworkId const &citizenId, std::string const &citizenProfessionSkillTemplate, int citizenLevel);
	static void addCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName);
	static void removeCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName);
	static bool hasCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName);
	static void getCitizenRank(int cityId, NetworkId const &citizenId, std::vector<std::string> &ranks);
	static void setCitizenTitle(int cityId, NetworkId const &citizenId, std::string const &citizenTitle);
	static void setCityStructureInfo(int cityId, NetworkId const &structureId, int structureType, bool structureValid);
	static void enterCityChatRoom(int cityId, CreatureObject const &who);
	static void reenterCityChatRoom(CreatureObject const &who);
	static void leaveCityChatRoom(int cityId, CreatureObject const &who);
	static void sendCityChat(CreatureObject const &who, Unicode::String const &text);
	static void onChatRoomCreate(std::string const &path);
	static void createAllCityChatRooms();
	static std::pair<int, std::string> mailToCitizens(CreatureObject const &who, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob);
	static std::pair<int, std::string> mailToCitizens(int cityId, Unicode::String const &subject, Unicode::String const &message, Unicode::String const &oob);
	static void verifyCitizenName(NetworkId const &citizenId, std::string const &citizenName);

	static void checkForDualCitizenship();

	static PgcRatingInfo const * getPgcRating(NetworkId const &chroniclerId);
	static void getPgcChroniclerId(std::string const &chroniclerName, std::vector<NetworkId> &chroniclerIds);
	static void adjustPgcRating(NetworkId const &chroniclerId, std::string const &chroniclerName, int adjustment);
	static void adjustPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int adjustment);
	static void setPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int value);
	static void verifyPgcChroniclerName(NetworkId const &chroniclerId, std::string const &chroniclerName);
};

// ======================================================================

#endif // _CityInterface_H_
