// ======================================================================
//
// ServerUniverse.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerUniverse_H
#define INCLUDED_ServerUniverse_H

#include "sharedFoundation/Timer.h"
#include "sharedGame/LfgCharacterData.h"
#include "sharedGame/Universe.h"
#include "sharedObject/CachedNetworkId.h"
#include <set>
#include <vector>

class AddResourceTypeMessage;
class CityObject;
class CreatureObject;
class GuildObject;
class GroupMemberParam;
class PlanetObject;
class PopulationList;
class ResourceClassObject;
class ResourcePoolObject;
class ResourceTypeObject;
class UniverseObject;

// ======================================================================

/**
* Singleton that manages Universe objects and functions that are global
* to the entire game universe.
*/
class ServerUniverse:public Universe
{
public:
	static ServerUniverse &getInstance();
	static void install();
	
public:
	bool                 isAuthoritative                () const;
	bool                 isUniverseLoaded               () const;
	GuildObject *        getMasterGuildObject           ();
	CityObject *         getMasterCityObject            ();
	PlanetObject *       getPlanetByName                (const std::string &sceneId) const;
	PlanetObject *       getCurrentPlanet               () const;
	PlanetObject *       getTatooinePlanet              () const;
	ResourceTypeObject * getResourceTypeByName          (std::string const & name) const;
	ResourceTypeObject * getResourceTypeById            (NetworkId const & id) const;
	ResourceTypeObject * getImportedResourceTypeById    (NetworkId const & id) const;
	void                 renameResourceType             (const std::string &oldname, const std::string &newname);
	int                  getNumNativeResourceTypes      () const;
	int                  getNumImportedResourceTypes    () const;
	PlanetObject *       createPlanetObject             (const std::string &planetName);
	void                 addUniverseObject              (UniverseObject &newObject);
	void                 requestCreateProxiesOnServer   (uint32 remoteProcess);
	void                 setUniverseProcess             (uint32 processId);
	uint32               getUniverseProcess             () const;
	void                 universeComplete               (uint32 sourceProcess);
	void                 universeLoadedAck              (uint32 sourceProcess);
	void                 registerMasterGuildObject      (GuildObject &masterGuildObject);
	void                 registerMasterCityObject       (CityObject &masterCityObject);
	void                 registerPlanetObject           (PlanetObject &newPlanet);
	void                 registerResourceTypeObject     (ResourceTypeObject &newType, bool newlySpawned);
	void                 sendUpdatedTypeToAllServers    (ResourceTypeObject &changedType);
	virtual void         resourceClassTreeLoaded        (std::map<int, ResourceClassObject*> &resourceClasses);
	std::string          generateRandomResourceName     (const std::string &nameTable) const;
	ResourceTypeObject const * pickRandomNonDepletedResource  (const std::string &parentResourceClassName) const;
//	int                  extractResourceFromRandomPool  (const std::string &parentResourceClassName, int amount, const Vector &pos);
	virtual ResourceClassObject* makeResourceClassObject();
	void                 createGroup                    (GroupMemberParam const & leader, std::vector<GroupMemberParam> const & members);
	void                 onServerConnectionClosed       (uint32 processId);
	void                 updatePopulationList           (const PopulationList &newData);
	int                  getPopulationAtLocation        (const std::string &scene, int x, int z) const;
	void                 update                         (float frameTime);
	void                 handleAddResourceTypeMessage   (AddResourceTypeMessage const & message);
	bool                 manualDepleteResource          (NetworkId const & resourceToDeplete);
	
	const NetworkId &    findTheaterId(const std::string & name);
	const std::string &  findTheaterName(const NetworkId & id);
	bool                 setTheater(const std::string & name, const NetworkId & id);
	void                 clearTheater(const std::string & name);
	bool                 remoteSetTheater(const std::string & name, const NetworkId & id);
	void                 remoteClearTheater(const std::string & name);

	void                 adjustGcwImperialScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int adjustment);
	void                 adjustGcwRebelScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int adjustment);
	int                  getGcwImperialScorePercentile(std::string const & gcwCategory) const; // the rebel score is 100 - the imperial score
	int                  getGcwGroupImperialScorePercentile(std::string const & gcwGroup) const; // the rebel score is 100 - the imperial score

	static void          setConnectedCharacterLfgData(const NetworkId & characterId, const LfgCharacterData & lfgCharacterData);
	static std::map<NetworkId, LfgCharacterData> const & getConnectedCharacterLfgData();

	static void          setConnectedCharacterGroupData(const NetworkId & characterId, const NetworkId & groupId);

	static void          setConnectedCharacterPlanetData(const NetworkId & characterId, const std::string & planetName);

	static void          setConnectedCharacterRegionData(const NetworkId & characterId, const std::string & regionName);

	static void          setConnectedCharacterPlayerCityData(const NetworkId & characterId, const std::string & playerCityName);

	static void          setConnectedCharacterLevelData(const NetworkId & characterId, int16 level);

	static void          setConnectedCharacterProfessionData(const NetworkId & characterId, LfgCharacterData::Profession profession);

	static void          setConnectedCharacterSearchableByCtsSourceGalaxyData(const NetworkId & characterId, bool searchableByCtsSourceGalaxy);

	static void          setConnectedCharacterDisplayLocationInSearchResultsData(const NetworkId & characterId, bool displayLocationInSearchResults);

	static void          setConnectedCharacterAnonymousData(const NetworkId & characterId, bool anonymous);

	static void          setConnectedCharacterInterestsData(const NetworkId & characterId, BitArray const & interests);

	static void          setConnectedCharacterBiographyData(const NetworkId & characterId, const Unicode::String & biography);
	static void          removeConnectedCharacterBiographyData(const NetworkId & characterId);
	static std::map<NetworkId, Unicode::String> const & getConnectedCharacterBiographyData();

	static void          removeConnectedCharacterData(const NetworkId & characterId);

	// Universe Wide Event Functions
	static void          addUniverseEvent(std::string const & eventName);
	static void          removeUniverseEvent(std::string const & eventName);
	static std::string const & getCurrentEvents();

protected:
	virtual void updateAndValidateData ();

protected:
	ServerUniverse();
	virtual ~ServerUniverse();

private:
	void                 createProxiesOnServer(std::vector<uint32> const & remoteProcesses);
	
private:
	//TODO:  consistency about whether STL containers are pointers or not
	typedef std::vector<CachedNetworkId*>             UniverseObjectList;
	UniverseObjectList * m_universeObjectList;
	UniverseObjectList * m_pendingUniverseObjects;
	bool m_loaded;
	bool m_authoritative;
	uint32 m_universeProcess;
	Timer m_nextCheckTimer;
	bool m_doImmediateCheck;
	
	//
	// Planet Objects
	//
	typedef std::map<const std::string, PlanetObject*> PlanetNameMap;
	
	PlanetObject  *m_thisPlanet;
	PlanetObject  *m_tatooinePlanet;
	PlanetNameMap *m_planetNameMap;
	
	//
	// Resource Type Objects
	//
	typedef std::map<std::string, ResourceTypeObject*>       ResourceTypeNameMap;
	typedef std::map<NetworkId, ResourceTypeObject*>         ResourceTypeIdMap;
	typedef std::vector<ResourceTypeObject*>                 ResourcesToSendType;
	
	ResourceTypeNameMap *m_resourceTypeNameMap;
	ResourceTypeIdMap *m_resourceTypeIdMap;
	ResourceTypeIdMap *m_importedResourceTypeIdMap;
	ResourcesToSendType *m_resourcesToSend;
	
	GuildObject *m_masterGuildObject;
	CityObject *m_masterCityObject;

	//	 UniverseObjectList  *m_pendingObjects;
	//	 bool m_loaded;

	// Theater data
	typedef std::map<const std::string, const NetworkId> TheaterNameIdMap;
	typedef std::map<const NetworkId, const std::string> TheaterIdNameMap;

	TheaterNameIdMap * m_theaterNameIdMap;
	TheaterIdNameMap * m_theaterIdNameMap;

	//
	// Non-object data about the universe
	//
	PopulationList *m_populationList;

private:
	typedef std::set<uint32> PendingProxyRequestList;
	PendingProxyRequestList m_pendingProxyRequests;

	typedef std::set<uint32> PendingUniverseLoadedAckList;
	PendingUniverseLoadedAckList m_pendingUniverseLoadedAckList;
	
	time_t m_timeUniverseDataSent;

private:
	ServerUniverse (ServerUniverse const &); //disable
	ServerUniverse & operator= (ServerUniverse const &); //disable
};

// ======================================================================

inline bool ServerUniverse::isAuthoritative() const
{
	return m_authoritative;
}

// ----------------------------------------------------------------------

/**
* Returns the PlanetObject representing the planet this server runs.
*/
inline PlanetObject *ServerUniverse::getCurrentPlanet() const
{
	WARNING(!m_thisPlanet,("getCurrentPlanet() was called, but there is no PlanetObject."));
	return m_thisPlanet;
}

// ----------------------------------------------------------------------

inline PlanetObject *ServerUniverse::getTatooinePlanet() const
{
	return m_tatooinePlanet;
}

// ----------------------------------------------------------------------

inline ServerUniverse &ServerUniverse::getInstance()
{
	return *(safe_cast<ServerUniverse*>(&Universe::getInstance()));
}

// ----------------------------------------------------------------------

inline int ServerUniverse::getNumNativeResourceTypes() const
{
	return m_resourceTypeIdMap->size();
}

// ----------------------------------------------------------------------

inline int ServerUniverse::getNumImportedResourceTypes() const
{
	return m_importedResourceTypeIdMap->size();
}

// ======================================================================

#endif
