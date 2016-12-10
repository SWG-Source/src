//========================================================================
//
// PlanetObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_PlanetObject_H
#define INCLUDED_PlanetObject_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaSetObserver.h"
#include "Archive/AutoDeltaVariableCallback.h"
#include "serverGame/UniverseObject.h"
#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/LfgCharacterData.h"
#include "sharedGame/TravelPoint.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedNetworkMessages/MapLocationArchive.h"

class CreatureObject;
class ResourceClassObject;
class ResourcePoolObject;
class ResourceTypeObject;
class ScriptParams;
class ServerPlanetObjectTemplate;
class Vector2d;

//-----------------------------------------------------------------------

/** 
 * A pool of a resource found on a planet.
 *
 * Represents the planet's supply of a particular resource.
 */

class PlanetObject : public UniverseObject
{
	friend class PlanetController;

public:

	PlanetObject(const ServerPlanetObjectTemplate* newTemplate);
	virtual ~PlanetObject();

	virtual Controller* createDefaultController (void);
	void                addMembersToPackages    ();
	virtual void        unload                  ();
	virtual void        onLoadedFromDatabase    ();

public:
	const std::string &    getName               () const;
	void                   setPlanetName         (const std::string &newName);
	virtual void           getAttributes         (std::vector<std::pair<std::string, Unicode::String> > &data) const;
		
	//resource pools functions
	void                   getAvailableResourceList (std::vector<ResourceTypeObject const *> &buffer) const;
	void                   getAvailableResourceList (std::vector<ResourceTypeObject const *> &buffer, const ResourceClassObject &masterClass) const;
	void                   debugOutputPools         (std::string &output) const;

	// weather functions
	void setWeather(int weather, float windVelocityX, float windVelocityY, float windVelocityZ);
	void getWeather(int & weather, float & windVelocityX, float & windVelocityY, float & windVelocityZ) const;

	const std::vector<std::pair<NetworkId, unsigned long> > & getSpawnQueue() const;
	public:
	// Universe system functions (rebuild data structures on load)
	virtual void  setupUniverse ();

	//-- planetary map support

	typedef std::vector<MapLocation>                    MapLocationVector;
	typedef std::map<NetworkId, MapLocation>            MapLocationMap;
	typedef Archive::AutoDeltaMap<NetworkId, MapLocation>  ArchiveMapLocationMap;
	typedef std::vector<int>                            IntVector;

	bool                            addMapLocation               (const MapLocation &location, int mapLocationType, bool enforceLocationCountLimits);
	bool                            removeMapLocation            (const NetworkId &id);
	bool                            removeMapLocation            (const NetworkId &id, int mapLocationType);
	const MapLocationVector &       getMapLocations              (int mapLocationType) const;
	void                            getMapLocations              (MapLocationVector & result) const;
	void                            getMapLocationsByCategories  (int mapLocationType, uint8 category, uint8 subCategory, MapLocationVector & result) const;
	int                             getMapLocationVersionForType (int mapLocationType) const;
	void                            getMapLocationCategories     (int parentCategory, int mapLocationType, IntVector & result) const;
	const MapLocation *             getMapLocation               (int mapLocationType, const NetworkId & id) const;

	//-- travel location support
	void               addTravelPoint (const std::string& name, const Vector& position_w, int cost, bool interplanetary, TravelPoint::TravelPointType type);
	void               removeTravelPoint (const std::string& name);
	int                getNumberOfTravelPoints () const;
	const TravelPoint* getTravelPoint (int travelPointIndex) const;
	const TravelPoint* getTravelPoint (const std::string& name) const;
	void               getTravelPointNameList (std::vector<std::string>& travelPointNameList) const;
	void               getTravelPointPointList (std::vector<Vector>& travelPointPointList) const;
	void               getTravelPointCostList (std::vector<int>& travelPointCostList) const;
	void               getTravelPointInterplanetaryList (std::vector<bool>& travelPointInterplanetaryList) const;

	std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > > const & getCollectionServerFirst() const;
	int                getCollectionServerFirstUpdateNumber() const;
	void               setCollectionServerFirst(const CollectionsDataTable::CollectionInfoCollection& collectionInfo, const NetworkId& claimantId, const Unicode::String& claimantName, const std::string& claimantAccountInfo);
	void               removeCollectionServerFirst(const CollectionsDataTable::CollectionInfoCollection& collectionInfo);

	void               setConnectedCharacterLfgData(const NetworkId & characterId, const LfgCharacterData & lfgCharacterData);
	std::map<NetworkId, LfgCharacterData> const & getConnectedCharacterLfgData() const;
	std::map<std::string, std::pair<int, int> > const & getConnectedCharacterLfgDataFactionalPresence() const;
	static             ScriptParams const *getConnectedCharacterLfgDataFactionalPresenceTableDictionary();

	void               setConnectedCharacterGroupData(const NetworkId & characterId, const NetworkId & groupId);

	void               setConnectedCharacterPlanetData(const NetworkId & characterId, const std::string & planetName);

	void               setConnectedCharacterRegionData(const NetworkId & characterId, const std::string & regionName);

	void               setConnectedCharacterPlayerCityData(const NetworkId & characterId, const std::string & playerCityName);

	void               setConnectedCharacterLevelData(const NetworkId & characterId, int16 level);

	void               setConnectedCharacterProfessionData(const NetworkId & characterId, LfgCharacterData::Profession profession);

	void               setConnectedCharacterSearchableByCtsSourceGalaxyData(const NetworkId & characterId, bool searchableByCtsSourceGalaxy);

	void               setConnectedCharacterDisplayLocationInSearchResultsData(const NetworkId & characterId, bool displayLocationInSearchResults);

	void               setConnectedCharacterAnonymousData(const NetworkId & characterId, bool anonymous);

	void               setConnectedCharacterInterestsData(const NetworkId & characterId, BitArray const & interests);

	void               setConnectedCharacterBiographyData(const NetworkId & characterId, const Unicode::String & biography);
	void               removeConnectedCharacterBiographyData(const NetworkId & characterId);
	std::map<NetworkId, Unicode::String> const & getConnectedCharacterBiographyData() const;

	void               removeConnectedCharacterData(const NetworkId & characterId);

	void                addUniverseEvent(std::string const & eventName);
	void                removeUniverseEvent(std::string const & eventName);
	std::string const & getCurrentEvents() const;
	void                parseCurrentEventsList(std::vector<std::string> & outVector);

	void               updateGcwTrackingData();

	void               adjustGcwImperialScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int64 adjustment);
	void               adjustGcwRebelScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int64 adjustment);
	int                getGcwImperialScorePercentile(std::string const & gcwCategory) const; // the rebel score is 100 - the imperial score

	std::map<std::string, std::pair<int64, int64> > const & getGcwImperialScore() const;
	std::map<std::string, std::pair<int64, int64> > const & getGcwRebelScore() const;

protected:
	virtual void       handleCMessageTo(const MessageToPayload &message);
	virtual void       endBaselines();

private:

	void               buildAvailableResourceList       () const;
	bool               getTravelPointIndex              (const std::string& name, int& travelPointIndex) const;

	void               cacheMapLocationsInternal        () const;
	void               incrementMapLocationCacheVersion (int mapLocationType);

	Archive::AutoDeltaVariable<int> &  getMapLocationVersionForTypeInternal     (int mapLocationType) const;
	int &                              getMapLocationListVersionForTypeInternal (int mapLocationType) const;
	const MapLocationVector &          getMapLocationListForType                (int mapLocationType) const;
	MapLocationVector &                getMapLocationListForTypeInternal        (int mapLocationType);

	const ArchiveMapLocationMap &      getMapLocationMapForType                 (int mapLocationType) const;
	ArchiveMapLocationMap &            getMapLocationMapForTypeInternal         (int mapLocationType);

	void                               checkUpdateInternalListVersion           (int mapLocationType) const;

	void connectedCharacterLfgDataFactionalPresenceOnErase(std::string const & keyValue, std::pair<int, int> const & value);
	void connectedCharacterLfgDataFactionalPresenceOnInsert(std::string const & keyValue, std::pair<int, int> const & value);
	void connectedCharacterLfgDataFactionalPresenceOnSet(std::string const & keyValue, std::pair<int, int> const & oldValue, std::pair<int, int> const & newValue);

private:
	struct EventsUpdatedCallback
	{
		void modified(PlanetObject &target, std::string oldValue, std::string value, bool isLocal);
	};

	PlanetObject();
	PlanetObject(const PlanetObject& rhs);
	PlanetObject&	operator=(const PlanetObject& rhs);

private:

// BPM PlanetObject : UniverseObject // Begin persisted members.
	Archive::AutoDeltaVariable<std::string>       m_planetName;      ///< The name of the planet.  Must match the sceneId of the gameservers that load objects on this planet.
//EPM

	//-- Resource data cache
	bool mutable   m_builtAvailableResourceList;
	uint32 mutable m_availableResourceListExpirationTime;
	std::vector<ResourceTypeObject const *> mutable m_availableResourceList;
	
	//-- travel location support
	Archive::AutoDeltaVector<TravelPoint>         m_travelPointList;

	// weather support
	Archive::AutoDeltaVariable<int>       m_weatherIndex;
	Archive::AutoDeltaVariable<float>     m_windVelocityX;
	Archive::AutoDeltaVariable<float>     m_windVelocityY;
	Archive::AutoDeltaVariable<float>     m_windVelocityZ;

	//-- planetary map support.  @todo remove this from PlanetObject

	ArchiveMapLocationMap    m_mapLocationMapStatic;     // static stuff like buildings & cities
	ArchiveMapLocationMap    m_mapLocationMapDynamic;    // dynamic stuff like camps
	ArchiveMapLocationMap    m_mapLocationMapPersist;    // semi-permanent stuff like player structures, vendors, etc...

	mutable Archive::AutoDeltaVariable<int>  m_mapLocationVersionStatic;  // version numbers for caching
	mutable Archive::AutoDeltaVariable<int>  m_mapLocationVersionDynamic; //
	mutable Archive::AutoDeltaVariable<int>  m_mapLocationVersionPersist; //

	//-- planetary map local cache.  The maps above are synchronized and persisted.  The vectors below are stored
	//-- and maintained on a per-instance basis.

	mutable MapLocationVector                m_mapLocationListStatic;     // static stuff like buildings & cities
	mutable MapLocationVector                m_mapLocationListDynamic;    // dynamic stuff like camps
	mutable MapLocationVector                m_mapLocationListPersist;    // semi-permanent stuff like player structures, vendors, etc...

	mutable int                              m_mapLocationListVersionStatic;  // version numbers for caching
	mutable int                              m_mapLocationListVersionDynamic; //
	mutable int                              m_mapLocationListVersionPersist; //

	struct CollectionServerFirstObserver
	{
		CollectionServerFirstObserver(PlanetObject *planet, Archive::AutoDeltaObserverOp operation);
		~CollectionServerFirstObserver();

		PlanetObject * m_target;
	};
	friend struct CollectionServerFirstObserver;

	Archive::AutoDeltaSetObserver<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> >, CollectionServerFirstObserver, PlanetObject> m_collectionServerFirst;
	Archive::AutoDeltaVariable<int>          m_collectionServerFirstUpdateNumber;

	Archive::AutoDeltaMap<NetworkId, LfgCharacterData> m_connectedCharacterLfgData;
	Archive::AutoDeltaMap<std::string, std::pair<int, int>, PlanetObject> m_connectedCharacterLfgDataFactionalPresence;
	Archive::AutoDeltaMap<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > > m_connectedCharacterLfgDataFactionalPresenceGrid;
	Archive::AutoDeltaMap<NetworkId, Unicode::String> m_connectedCharacterBiographyData;

	// Event String
	Archive::AutoDeltaVariableCallback<std::string, EventsUpdatedCallback, PlanetObject> m_currentEvents;

	unsigned long m_nextGcwTrackingUpdate;
	std::map<std::string, int64> m_gcwImperialScoreAdjustment;
	std::map<std::string, int64> m_gcwRebelScoreAdjustment;

	// the score is tracked as a <cumulative score, current period score> pair
	Archive::AutoDeltaMap<std::string, std::pair<int64, int64> > m_gcwImperialScore;
	Archive::AutoDeltaMap<std::string, std::pair<int64, int64> > m_gcwRebelScore;
};

// ----------------------------------------------------------------------

inline const std::string &PlanetObject::getName() const
{
	return m_planetName.get();
}

inline void PlanetObject::getWeather(int & weather, float & windVelocityX, 
	float & windVelocityY, float & windVelocityZ) const
{
	weather = m_weatherIndex.get();
	windVelocityX = m_windVelocityX.get();
	windVelocityY = m_windVelocityY.get();
	windVelocityZ = m_windVelocityZ.get();
}

inline std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > > const & PlanetObject::getCollectionServerFirst() const
{
	return m_collectionServerFirst.get();
}

inline int PlanetObject::getCollectionServerFirstUpdateNumber() const
{
	return m_collectionServerFirstUpdateNumber.get();
}

inline std::map<NetworkId, LfgCharacterData> const & PlanetObject::getConnectedCharacterLfgData() const
{
	return m_connectedCharacterLfgData.getMap();
}

inline std::map<std::string, std::pair<int, int> > const & PlanetObject::getConnectedCharacterLfgDataFactionalPresence() const
{
	return m_connectedCharacterLfgDataFactionalPresence.getMap();
}

inline std::map<NetworkId, Unicode::String> const & PlanetObject::getConnectedCharacterBiographyData() const
{
	return m_connectedCharacterBiographyData.getMap();
}

inline std::map<std::string, std::pair<int64, int64> > const & PlanetObject::getGcwImperialScore() const
{
	return m_gcwImperialScore.getMap();
}

inline std::map<std::string, std::pair<int64, int64> > const & PlanetObject::getGcwRebelScore() const
{
	return m_gcwRebelScore.getMap();
}

//-----------------------------------------------------------------------
#endif	// INCLUDED_PlanetObject_H
