//========================================================================
//
// CityObject.h
//
// Copyright 2003 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_CityObject_H
#define INCLUDED_CityObject_H

// ======================================================================

#include "Archive/AutoDeltaSet.h"
#include "serverGame/CitizenInfo.h"
#include "serverGame/CityInfo.h"
#include "serverGame/CityStructureInfo.h"
#include "serverGame/PgcRatingInfo.h"
#include "serverGame/UniverseObject.h"

// ======================================================================

class ServerCityObjectTemplate;

// ======================================================================

class CityObject: public UniverseObject
{
public:
	CityObject(ServerCityObjectTemplate const *newTemplate);
	virtual ~CityObject();

	virtual Controller* createDefaultController();
	virtual void setupUniverse();
	virtual void getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const;
	virtual void unload();

	int createCity(std::string const &cityName, NetworkId const &cityHallId, std::string const &cityPlanet, int x, int z, int radius, NetworkId const &leaderId, int incomeTax, int propertyTax, int salesTax, Vector const &travelLoc, int travelCost, bool travelInterplanetary, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId, int cityId = 0);
	void removeCity(int cityId);
	void removeCitizen(int cityId, NetworkId const &citizenId);
	void removeCityStructure(int cityId, NetworkId const &structureId);
	void setCityName(int cityId, std::string const &cityName);
	void setCityHall(int cityId, NetworkId const &cityHallId);
	void setCityLocation(int cityId, std::string const &cityPlanet, int x, int z);
	void setCityRadius(int cityId, int radius);
	void setCityFaction(int cityId, uint32 faction, bool notifyCitizens);
	void setCityGcwDefenderRegion(int cityId, std::string const &gcwDefenderRegion, int timeJoined, bool notifyCitizens);
	void setCityCreationTime(int cityId, int creationTime);
	void setCityLeader(int cityId, NetworkId const &leaderId);
	void setCityIncomeTax(int cityId, int incomeTax);
	void setCityPropertyTax(int cityId, int propertyTax);
	void setCitySalesTax(int cityId, int salesTax);
	void setCityTravelInfo(int cityId, Vector const &travelLoc, int travelCost, bool travelInterplanetary);
	void setCityCloneInfo(int cityId, Vector const &cloneLoc, Vector const &cloneRespawn, NetworkId const &cloneRespawnCell, NetworkId const &cloneId);
	void setCitizen(int cityId, NetworkId const &citizenId, std::string const &citizenName, NetworkId const &allegiance, int permissions);
	void setCitizenProfessionInfo(int cityId, NetworkId const &citizenId, std::string const &citizenProfessionSkillTemplate, int citizenLevel);
	void addCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName);
	void removeCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName);
	bool hasCitizenRank(int cityId, NetworkId const &citizenId, std::string const &rankName);
	void getCitizenRank(int cityId, NetworkId const &citizenId, std::vector<std::string> &ranks);
	void setCitizenTitle(int cityId, NetworkId const &citizenId, std::string const &citizenTitle);
	void setCityStructure(int cityId, NetworkId const &structureId, int structureType, bool structureValid);

	bool cityExists(int cityId) const;
	CityInfo const &getCityInfo(int cityId) const;
	void getAllCityIds(std::vector<int> &results) const;
	std::map<int, CityInfo> const &getAllCityInfo() const;
	int findCityByName(std::string const &name) const;
	int findCityByCityHall(NetworkId const &cityHallId) const;
	int getCityAtLocation(std::string const &planetName, int x, int z, int radius) const;
	void getCitizenIds(int cityId, std::vector<NetworkId> &results) const;
	std::map<std::pair<int, NetworkId>, CitizenInfo> const &getAllCitizensInfo() const;
	CitizenInfo const *getCitizenInfo(int cityId, NetworkId const &citizenId) const;
	void getCityStructureIds(int cityId, std::vector<NetworkId> &results) const;
	std::map<std::pair<int, NetworkId>, CityStructureInfo> const & getAllCityStructuresInfo() const;
	CityStructureInfo const *getCityStructureInfo(int cityId, NetworkId const &structureId) const;
	std::vector<int> const & getCitizenOfCityId(NetworkId const &citizenId) const;

	std::map<std::pair<std::string, int>, uint32> const & getGcwRegionDefenderCities() const;
	std::map<std::string, std::pair<int, int> > const & getGcwRegionDefenderCitiesCount() const;
	int getGcwRegionDefenderCitiesVersion() const;

	void checkForDualCitizenship() const;

	PgcRatingInfo const * getPgcRating(NetworkId const &chroniclerId) const;
	void getPgcChroniclerId(std::string const &chroniclerName, std::vector<NetworkId> &chroniclerIds) const;
	void adjustPgcRating(NetworkId const &chroniclerId, std::string const &chroniclerName, int adjustment);
	void adjustPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int adjustment);
	void setPgcRatingData(NetworkId const &chroniclerId, std::string const &chroniclerName, int index, int value);
	void verifyPgcChroniclerName(NetworkId const &chroniclerId, std::string const &chroniclerName);

private:
	CityObject();
	CityObject(CityObject const &rhs);
	CityObject& operator=(CityObject const &rhs);

	void addMembersToPackages();
	int nextFreeCityId();
	void getCitySpec(int cityId, std::string &result) const;
	CitizenInfo const * getCitizenSpec(int cityId, NetworkId const &citizenId, std::string &result) const;
	CityStructureInfo const * getCityStructureSpec(int cityId, NetworkId const &structureId, std::string &result) const;
	void setCitySpec(int cityId, std::string const &newCitySpec);
	bool findCitySpec(int cityId, std::string &citySpec) const;
	void setCitizen(std::string const &oldCitizenSpec, std::string const &newCitizenSpec);

	void addGcwRegionDefenderCity(std::string const & gcwRegion, int cityId, uint32 faction);
	void removeGcwRegionDefenderCity(std::string const & gcwRegion, int cityId, uint32 faction);

	void onCitizensInfoErase(std::pair<int, NetworkId> const & key, CitizenInfo const & value);
	void onCitizensInfoInsert(std::pair<int, NetworkId> const & key, CitizenInfo const & value);
	void onCitizensInfoSet(std::pair<int, NetworkId> const & key, CitizenInfo const & oldValue, CitizenInfo const & newValue);

private:
// BPM CityObject : UniverseObject // Begin persisted members
	Archive::AutoDeltaSet<std::string> m_cities;
	Archive::AutoDeltaSet<std::string> m_citizens;
	Archive::AutoDeltaSet<std::string> m_structures;
// EPM

	// parallel maps to persisted members m_cities, m_citizens, and m_structures that
	// will provide access to the data because the data in the persisted members are strings
	Archive::AutoDeltaMap<int, CityInfo> m_citiesInfo; // city id -> CityInfo
	Archive::AutoDeltaMap<std::pair<int, NetworkId>, CitizenInfo, CityObject> m_citizensInfo; // (city id, citizen id) -> CitizenInfo
	Archive::AutoDeltaMap<std::pair<int, NetworkId>, CityStructureInfo> m_structuresInfo; // (city id, structure id) -> CityStructureInfo
	Archive::AutoDeltaMap<NetworkId, std::vector<int> > m_citizenToCityId; // citizen id -> city id (could be multiple for the bug where a character is a citizen of multiple cities)

	Archive::AutoDeltaMap<NetworkId, PgcRatingInfo> m_pgcRatingInfo;
	Archive::AutoDeltaSet<std::pair<std::string, NetworkId> > m_pgcRatingChroniclerId; // <lowercase chronicler first name, chronicler id>

	Archive::AutoDeltaMap<std::pair<std::string, int>, uint32> m_gcwRegionDefenderCities;
	Archive::AutoDeltaMap<std::string, std::pair<int, int> > m_gcwRegionDefenderCitiesCount;
	Archive::AutoDeltaVariable<int> m_gcwRegionDefenderCitiesVersion;

	int m_lowFreeCityId;
};

// ----------------------------------------------------------------------

inline std::map<std::pair<std::string, int>, uint32> const & CityObject::getGcwRegionDefenderCities() const
{
	return m_gcwRegionDefenderCities.getMap();
}

// ----------------------------------------------------------------------

inline std::map<std::string, std::pair<int, int> > const & CityObject::getGcwRegionDefenderCitiesCount() const
{
	return m_gcwRegionDefenderCitiesCount.getMap();
}

// ----------------------------------------------------------------------

inline int CityObject::getGcwRegionDefenderCitiesVersion() const
{
	return m_gcwRegionDefenderCitiesVersion.get();
}

// ======================================================================

#endif
