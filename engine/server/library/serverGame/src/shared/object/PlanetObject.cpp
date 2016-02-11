//========================================================================
//
// PlanetObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlanetObject.h"

#include "UnicodeUtils.h"
#include "boost/smart_ptr.hpp"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/GuildObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ObjectIdManager.h"
#include "serverGame/PlanetController.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/Pvp.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerBuildoutManager.h"
#include "serverGame/ServerPlanetObjectTemplate.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableListNestedList.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedGame/PvpData.h"
#include "sharedGame/TravelPoint.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedNetworkMessages/MapLocationArchive.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgServerNetworkMessages/MessageQueuePlanetAddTravelPoint.h"

#include <algorithm>
#include <vector>

//----------------------------------------------------------------------

namespace PlanetObjectNamespace
{
	const int s_maxMapLocationsStatic  = 1000;
	const int s_maxMapLocationsDynamic = 1000;
	const int s_maxMapLocationsPersist = 1000;

	// objvars for collection "server first" tracking
	const std::string OBJVAR_COLLECTION_SERVER_FIRST("collectionServerFirst");
	const std::string OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER("collectionServerFirstUpdateNumber");

	typedef std::pair<std::string, std::string::size_type> StringIndexPair;

	void ParseEventString(std::string const & eventString, std::vector<std::string> & parsedVector)
	{
		std::string delimiter = ",";

		std::string::size_type index = eventString.find(delimiter);
		std::string::size_type offset = 0;

		parsedVector.clear();
		
		if(eventString.empty())
			return;

		if(index == std::string::npos)
		{
			parsedVector.push_back(eventString);
			return;
		}

		while(index != std::string::npos)
		{
			std::string subString = eventString.substr(offset, (index - offset));
			parsedVector.push_back(subString);
			offset = index + 1;
			index = eventString.find(delimiter, offset);
		}

		std::string subString = eventString.substr(offset, eventString.size());
		parsedVector.push_back(subString);
	}
	
	void ParseEventString(std::string const & eventString, std::vector<StringIndexPair> & parsedVector)
	{
		std::string delimiter = ",";

		std::string::size_type index = eventString.find(delimiter);
		std::string::size_type offset = 0;

		parsedVector.clear();

		if(eventString.empty())
			return;

		if(index == std::string::npos)
		{
			parsedVector.push_back(std::make_pair(eventString, 0));
			return;
		}

		while(index != std::string::npos)
		{
			std::string subString = eventString.substr(offset, (index - offset));
			parsedVector.push_back(std::make_pair(subString, index));
			offset = index + 1;
			index = eventString.find(delimiter, offset);
		}

		std::string subString = eventString.substr(offset, eventString.size());
		parsedVector.push_back(std::make_pair(subString, index));
	}

	const char * makeCopyOfString(const char * rhs)
	{
		char * lhs = nullptr;
		if (rhs)
		{
			lhs = new char[strlen(rhs) + 1];
			strcpy(lhs, rhs);
		}
		else
		{
			lhs = new char[1];
			lhs[0] = '\0';
		}

		return lhs;
	}

	int getNextGcwScoreDecayTime(int base);

	// dictionary containing the table showing factional presence activity
	ScriptParams * s_factionalPresenceSuiTable = nullptr;
	bool s_factionalPresenceSuiTableNeedRebuild = true;

	void decrementConnectedCharacterLfgDataFactionalPresenceCount(Archive::AutoDeltaMap<std::string, std::pair<int, int>, PlanetObject> & connectedCharacterLfgDataFactionalPresence, const LfgCharacterData & lfgCharacterData);
	void incrementConnectedCharacterLfgDataFactionalPresenceCount(Archive::AutoDeltaMap<std::string, std::pair<int, int>, PlanetObject> & connectedCharacterLfgDataFactionalPresence, const LfgCharacterData & lfgCharacterData);

	void decrementConnectedCharacterLfgDataFactionalPresenceGridCount(Archive::AutoDeltaMap<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > > & connectedCharacterLfgDataFactionalPresenceGrid, const LfgCharacterData & lfgCharacterData);
	void incrementConnectedCharacterLfgDataFactionalPresenceGridCount(Archive::AutoDeltaMap<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > > & connectedCharacterLfgDataFactionalPresenceGrid, const LfgCharacterData & lfgCharacterData);

	void removeFactionalPresenceGridFromPlanetMap(std::string const & planetName, NetworkId const & locationId);
	void addFactionalPresenceGridToPlanetMap(std::string const & planetName, NetworkId const & locationId, int gridX, int gridZ, bool imperial, bool rebel);

#ifdef _DEBUG
	bool ms_printEventDebug = false;
#endif
}

using namespace PlanetObjectNamespace;

//-----------------------------------------------------------------------

PlanetObject::PlanetObject(const ServerPlanetObjectTemplate* newTemplate) : 
	UniverseObject(newTemplate),
	m_planetName(),
	m_builtAvailableResourceList(false),
	m_availableResourceListExpirationTime(0),
	m_availableResourceList(),
	m_travelPointList (),
	m_weatherIndex(0),
	m_windVelocityX(0.0f),
	m_windVelocityY(0.0f),
	m_windVelocityZ(0.0f),
	m_mapLocationMapStatic          (),
	m_mapLocationMapDynamic         (),
	m_mapLocationMapPersist         (),

	// reserve version number 0, so that the client can use 0 to force an update
	m_mapLocationVersionStatic      (1),
	m_mapLocationVersionDynamic     (1),
	m_mapLocationVersionPersist     (1),

	m_mapLocationListStatic         (),
	m_mapLocationListDynamic        (),
	m_mapLocationListPersist        (),

	// reserve version number 0, so that the client can use 0 to force an update
	m_mapLocationListVersionStatic  (1),
	m_mapLocationListVersionDynamic (1),
	m_mapLocationListVersionPersist (1),

	// used to track changes to the collection system "server first" tracking system
	m_collectionServerFirst         (),
	m_collectionServerFirstUpdateNumber(0),

	// information tracked for each connected character
	m_connectedCharacterLfgData     (),
	m_connectedCharacterLfgDataFactionalPresence(),
	m_connectedCharacterLfgDataFactionalPresenceGrid(),
	m_connectedCharacterBiographyData(),
	
	m_currentEvents                 (),

	m_nextGcwTrackingUpdate         (0),
	m_gcwImperialScoreAdjustment    (),
	m_gcwRebelScoreAdjustment       (),
	m_gcwImperialScore              (),
	m_gcwRebelScore                 ()
{
	addMembersToPackages();

	m_collectionServerFirst.setSourceObject(this);
	m_currentEvents.setSourceObject(this);

	m_connectedCharacterLfgDataFactionalPresence.setOnErase(this, &PlanetObject::connectedCharacterLfgDataFactionalPresenceOnErase);
	m_connectedCharacterLfgDataFactionalPresence.setOnInsert(this, &PlanetObject::connectedCharacterLfgDataFactionalPresenceOnInsert);
	m_connectedCharacterLfgDataFactionalPresence.setOnSet(this, &PlanetObject::connectedCharacterLfgDataFactionalPresenceOnSet);
}

//-----------------------------------------------------------------------

PlanetObject::~PlanetObject()
{
}

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* PlanetObject::createDefaultController(void)
{
	Controller* controller = new PlanetController(this);

	setController(controller);
	return controller;
}	// PlanetObject::createDefaultController

// ----------------------------------------------------------------------

void PlanetObject::debugOutputPools(std::string &output) const
{
	buildAvailableResourceList();

	output+=m_planetName.get();
	output+='\n';
		
	for (std::vector<ResourceTypeObject const *>::const_iterator i=m_availableResourceList.begin(); i!=m_availableResourceList.end(); ++i)
	{
		if (*i)
			(*i)->debugOutput(output);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setupUniverse()
{
	ServerUniverse::getInstance().registerPlanetObject(*this);
}

// ----------------------------------------------------------------------

/**
 * Returns a list of all the non-depleted resources that have a pool on this planet
 */
void PlanetObject::getAvailableResourceList(std::vector<ResourceTypeObject const *> &buffer) const
{
	buildAvailableResourceList();
	
	buffer = m_availableResourceList;
}

// ----------------------------------------------------------------------

/**
 * Returns a list of all non-depeleted resources that have a pool on this
 * planet and are derived from the specified class
 */
void PlanetObject::getAvailableResourceList(std::vector<ResourceTypeObject const *> &buffer, const ResourceClassObject &masterClass) const
{
	buildAvailableResourceList();

	buffer.clear();
	
	for (std::vector<ResourceTypeObject const *>::const_iterator i=m_availableResourceList.begin(); i!=m_availableResourceList.end(); ++i)
	{
		if (*i && (*i)->isDerivedFrom(masterClass))
		{
			buffer.push_back(*i);
		}
	}	
}

// ----------------------------------------------------------------------

void PlanetObject::setPlanetName(const std::string &newName)
{
	if (isAuthoritative())
	{
		m_planetName=newName;
	}
	else
	{
		//TODO: message
	}
}

//-----------------------------------------------------------------------

/**
 * Sets the weather for the planet.
 */
void PlanetObject::setWeather(int weather, float windVelocityX, float windVelocityY, float windVelocityZ)
{
	if (isAuthoritative())
	{
		m_weatherIndex = weather;
		m_windVelocityX = windVelocityX;
		m_windVelocityY = windVelocityY;
		m_windVelocityZ = windVelocityZ;
	}
	else
	{
		sendControllerMessageToAuthServer(CM_setWeather, 
			new MessageQueueGenericValueType<std::pair<int, Vector> >(
			std::make_pair(weather, Vector(windVelocityX, windVelocityY, windVelocityZ))));
	}
}

// ----------------------------------------------------------------------

void PlanetObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	UniverseObject::getAttributes(data);
}

// ----------------------------------------------------------------------

void PlanetObject::addTravelPoint (const std::string& name, const Vector& position_w, const int cost, const bool interplanetary, const TravelPoint::TravelPointType type)
{
	if (ConfigServerGame::getDisableTravel())
	{
		//travel in a planet is off.
		return;
	}
	if (isAuthoritative ())
	{
		if (getTravelPoint (name))
		{
			DEBUG_WARNING (true, ("PlanetObject::addTravelPoint: adding existing travel point %s for planet %s", name.c_str (), getName ().c_str ()));
			return;
		}

		m_travelPointList.push_back (TravelPoint (name, Pvp::getGcwScoreCategoryRegion(m_planetName.get(), position_w), position_w, cost, interplanetary, type));

		DEBUG_REPORT_LOG_PRINT (true, ("PlanetObject::addTravelPoint: adding travel point %s for planet %s at position <%1.2f, %1.2f, %1.2f>\n", name.c_str (), getName ().c_str (), position_w.x, position_w.y, position_w.z));
	}
	else
	{
		Controller* const controller = getController ();
		if (!controller)
		{
			DEBUG_WARNING (true, ("PlanetObject::addTravelPoint: planet object %s has no controller", getName ().c_str ()));
			return;
		}

		MessageQueuePlanetAddTravelPoint* const message = new MessageQueuePlanetAddTravelPoint (name, position_w, cost, interplanetary, static_cast<uint32>(type));
		getController ()->appendMessage (static_cast<int>(CM_planetAddTravelPoint), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

		DEBUG_REPORT_LOG_PRINT (true, ("PlanetObject::addTravelPoint: sending add travel point %s for planet %s at position <%1.2f, %1.2f, %1.2f> to authoritative server\n", name.c_str (), getName ().c_str (), position_w.x, position_w.y, position_w.z));
	}
}

// ----------------------------------------------------------------------

void PlanetObject::removeTravelPoint (const std::string& name)
{
	if (isAuthoritative ())
	{
		int travelPointIndex;
		if (!getTravelPointIndex (name, travelPointIndex))
		{
			DEBUG_WARNING (true, ("PlanetObject::removeTravelPoint: attempting to remove non-existant travel point %s for planet %s", name.c_str (), getName ().c_str ()));
			return;
		}

		m_travelPointList.erase (static_cast<uint> (travelPointIndex));

		DEBUG_REPORT_LOG_PRINT (true, ("PlanetObject::removeTravelPoint: removing travel point %s for planet %s\n", name.c_str (), getName ().c_str ()));
	}
	else
	{
		Controller* const controller = getController ();
		if (!controller)
		{
			DEBUG_WARNING (true, ("PlanetObject::removeTravelPoint: planet object %s has no controller", getName ().c_str ()));
			return;
		}

		MessageQueueGenericValueType<std::string>* const message = new MessageQueueGenericValueType<std::string> (std::string (name));
		controller->appendMessage (static_cast<int>(CM_planetRemoveTravelPoint), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);

		DEBUG_REPORT_LOG_PRINT (true, ("PlanetObject::removeTravelPoint: sending remove travel point %s for planet %s to authoritative server\n", name.c_str (), getName ().c_str ()));
	}
}

// ----------------------------------------------------------------------

int PlanetObject::getNumberOfTravelPoints () const
{
	return static_cast<int>(m_travelPointList.size ());
}

// ----------------------------------------------------------------------

const TravelPoint* PlanetObject::getTravelPoint (const int travelPointIndex) const
{
	if (travelPointIndex < 0 || travelPointIndex >= getNumberOfTravelPoints ())
		return 0;

	return &m_travelPointList [static_cast<uint> (travelPointIndex)];
}

// ----------------------------------------------------------------------

const TravelPoint* PlanetObject::getTravelPoint (const std::string& name) const
{
	int i;
	for (i = 0; i < getNumberOfTravelPoints (); ++i)
	{
		const TravelPoint* const travelPoint = getTravelPoint (i);

		if (name == travelPoint->getName ())
			return travelPoint;
	}

	return 0;
}

// ----------------------------------------------------------------------

bool PlanetObject::getTravelPointIndex (const std::string& name, int& travelPointIndex) const
{
	int i;
	for (i = 0; i < getNumberOfTravelPoints (); ++i)
	{
		const TravelPoint* const travelPoint = getTravelPoint (i);

		if (name == travelPoint->getName ())
		{
			travelPointIndex = i;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void PlanetObject::getTravelPointNameList (std::vector<std::string>& travelPointNameList) const
{
	int i;
	for (i = 0; i < getNumberOfTravelPoints (); ++i)
	{
		const TravelPoint* const travelPoint = getTravelPoint (i);

		travelPointNameList.push_back (travelPoint->getName ());
	}
}

// ----------------------------------------------------------------------

void PlanetObject::getTravelPointPointList (std::vector<Vector>& travelPointPointList) const
{
	int i;
	for (i = 0; i < getNumberOfTravelPoints (); ++i)
	{
		const TravelPoint* const travelPoint = getTravelPoint (i);

		travelPointPointList.push_back (travelPoint->getPosition_w ());
	}
}

// ----------------------------------------------------------------------

void PlanetObject::getTravelPointCostList (std::vector<int>& travelPointCostList) const
{
	int i;
	for (i = 0; i < getNumberOfTravelPoints (); ++i)
	{
		const TravelPoint* const travelPoint = getTravelPoint (i);

		travelPointCostList.push_back (travelPoint->getCost ());
	}
}

// ----------------------------------------------------------------------

void PlanetObject::getTravelPointInterplanetaryList (std::vector<bool>& travelPointInterplanetaryList) const
{
	int i;
	for (i = 0; i < getNumberOfTravelPoints (); ++i)
	{
		const TravelPoint* const travelPoint = getTravelPoint (i);

		travelPointInterplanetaryList.push_back (travelPoint->getInterplanetary ()); //lint !e1025 !e1703 // lint can't parse this line
	}
}

//----------------------------------------------------------------------
//-- Map Location Stuff
//----------------------------------------------------------------------

//----------------------------------------------------------------------

bool PlanetObject::addMapLocation (const MapLocation &location, const int mapLocationType, const bool enforceLocationCountLimits)
{
	if (!location.m_category)
	{
		WARNING (true, ("PlanetObject can't add map location with zero category"));
		return false;
	}

	if (!location.m_locationId.isValid ())
	{
		WARNING (true, ("PlanetObject can't add map location with invalid id"));
		return false;
	}

	if (location.m_locationName.empty ())
	{
		WARNING (true, ("PlanetObject can't add map location with empty name"));
		return false;
	}

	if (!isAuthoritative ())
	{
		sendControllerMessageToAuthServer(CM_serverAddPlanetMapLocation, new MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > >(std::make_pair(location, std::make_pair(mapLocationType, (enforceLocationCountLimits ? static_cast<int>(1) : static_cast<int>(0))))));
		return true;
	}

	int limit = 0;
	switch (mapLocationType)
	{
	case PlanetMapManager::MLT_static:
		limit = s_maxMapLocationsStatic;
		break;
	case PlanetMapManager::MLT_dynamic:
		limit = s_maxMapLocationsDynamic;
		break;
	case PlanetMapManager::MLT_persist:
		limit = s_maxMapLocationsPersist;
		break;
	default:
		break;
	}

	ArchiveMapLocationMap & locationMap = getMapLocationMapForTypeInternal (mapLocationType);	

	//-- see if we are adding a new location, or just overwriting it in place

	MapLocationMap::const_iterator it = locationMap.find (location.m_locationId);
	if (it == locationMap.end ())
	{
		if (enforceLocationCountLimits && (static_cast<int>(locationMap.size ()) >= limit))
		{
			WARNING (true, ("PlanetObject limit [%d] reached for type [%d], can't add map location [%s,%s,%d,%d]", 
				limit, mapLocationType, location.m_locationId.getValueString ().c_str (), Unicode::wideToNarrow (location.m_locationName).c_str (), location.m_category, location.m_subCategory));
			return false;
		}
	}

	locationMap.set (location.m_locationId, location);

	//-- remove the location if it exists on another list
	{
		for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
		{
			if (i != mapLocationType)
				IGNORE_RETURN(removeMapLocation (location.m_locationId, i));
		}
	}

	incrementMapLocationCacheVersion (mapLocationType);

	return true;
}

//----------------------------------------------------------------------

bool PlanetObject::removeMapLocation (const NetworkId &id, int mapLocationType)
{
	if (!isAuthoritative ())
	{
		WARNING (true, ("PlanetObject::removeMapLocation (const NetworkId &id, int mapLocationType) invalid from non-auth planet object"));
		return false;
	}

	ArchiveMapLocationMap & locationMap = getMapLocationMapForTypeInternal (mapLocationType);

	if (locationMap.find (id) != locationMap.end ())
	{
		IGNORE_RETURN(locationMap.erase (id));
		incrementMapLocationCacheVersion (mapLocationType);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool PlanetObject::removeMapLocation (const NetworkId &id)
{
	if (!isAuthoritative ())
	{
		sendControllerMessageToAuthServer(CM_serverRemovePlanetMapLocation, new MessageQueueGenericValueType<NetworkId>(id));
		return true;
	}

	bool found = false;

	for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
	{
		if (removeMapLocation (id, i))
			found = true;
	}
	
	return found;
}

//----------------------------------------------------------------------

void PlanetObject::getMapLocations              (MapLocationVector & result) const
{
	for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
	{
		const MapLocationVector & mlv = getMapLocations (i);
		result.insert (result.end (), mlv.begin (), mlv.end ());
	}
}

//----------------------------------------------------------------------

const PlanetObject::MapLocationVector & PlanetObject::getMapLocations (int mapLocationType) const
{
	const MapLocationVector & locationList = getMapLocationListForType (mapLocationType);
	return locationList;
}

//----------------------------------------------------------------------

void PlanetObject::getMapLocationsByCategories (int mapLocationType, uint8 category, uint8 subCategory, MapLocationVector & result) const
{
	const MapLocationVector & locationList = getMapLocationListForType (mapLocationType);
	
	for (MapLocationVector::const_iterator i = locationList.begin (); i != locationList.end (); ++i)
	{
		const MapLocation & ml = *i;
		if (ml.getCategory() == category && ml.getSubCategory() == subCategory)
		{
			result.push_back (ml);
		}
	}
}

//----------------------------------------------------------------------

void PlanetObject::incrementMapLocationCacheVersion (int mapLocationType)
{
	if (!isAuthoritative ())
	{
		WARNING (true, ("PlanetObject::incrementMapLocationCacheVersion invalid from non-auth planet object"));
		return;
	}

	Archive::AutoDeltaVariable<int> & var = getMapLocationVersionForTypeInternal (mapLocationType);
	
	// reserve version number 0, so that the client can use 0 to force an update
	int version = var.get ();
	if (++version == 0)
		++version;

	var.set (version);
} //lint !e1762 // function looks like it could be made const, but better not to make it const because it uses a reference to set a member variable

//----------------------------------------------------------------------

int & PlanetObject::getMapLocationListVersionForTypeInternal (int mapLocationType) const
{
	switch (mapLocationType)
	{
	case PlanetMapManager::MLT_static:
		return m_mapLocationListVersionStatic;
	case PlanetMapManager::MLT_dynamic:
		return m_mapLocationListVersionDynamic;
	case PlanetMapManager::MLT_persist:
		return m_mapLocationListVersionPersist;
	default:
		break;
	}

	FATAL (true, ("PlanetObject::getMapLocationVersionForType failed"));
	static int broken; //lint !e527 // unreachable
	return broken;
}

//----------------------------------------------------------------------

int PlanetObject::getMapLocationVersionForType (int mapLocationType) const
{
	return const_cast<PlanetObject *>(this)->getMapLocationVersionForTypeInternal (mapLocationType).get ();
}

//----------------------------------------------------------------------

Archive::AutoDeltaVariable<int> & PlanetObject::getMapLocationVersionForTypeInternal (int mapLocationType) const
{
	switch (mapLocationType)
	{
	case PlanetMapManager::MLT_static:
		return m_mapLocationVersionStatic;
	case PlanetMapManager::MLT_dynamic:
		return m_mapLocationVersionDynamic;
	case PlanetMapManager::MLT_persist:
		return m_mapLocationVersionPersist;
	default:
		break;
	}

	FATAL (true, ("PlanetObject::getMapLocationVersionForType failed"));
	static Archive::AutoDeltaVariable<int> broken;  //lint !e527 // unreachable
	return broken;
}

//----------------------------------------------------------------------

const PlanetObject::MapLocationVector & PlanetObject::getMapLocationListForType    (int mapLocationType) const
{
	checkUpdateInternalListVersion (mapLocationType);
	return const_cast<PlanetObject *>(this)->getMapLocationListForTypeInternal (mapLocationType);
}

//----------------------------------------------------------------------

PlanetObject::MapLocationVector & PlanetObject::getMapLocationListForTypeInternal    (int mapLocationType)
{
	switch (mapLocationType)
	{
	case PlanetMapManager::MLT_static:
		return m_mapLocationListStatic;
	case PlanetMapManager::MLT_dynamic:
		return m_mapLocationListDynamic;
	case PlanetMapManager::MLT_persist:
		return m_mapLocationListPersist;
	default:
		break;
	}
	static MapLocationVector broken;
	return broken;
}

//----------------------------------------------------------------------

const PlanetObject::ArchiveMapLocationMap &      PlanetObject::getMapLocationMapForType                (int mapLocationType) const
{
	return const_cast<PlanetObject *>(this)->getMapLocationMapForTypeInternal (mapLocationType);
}

//----------------------------------------------------------------------

PlanetObject::ArchiveMapLocationMap &            PlanetObject::getMapLocationMapForTypeInternal        (int mapLocationType)
{
	switch (mapLocationType)
	{
	case PlanetMapManager::MLT_static:
		return m_mapLocationMapStatic;
	case PlanetMapManager::MLT_dynamic:
		return m_mapLocationMapDynamic;
	case PlanetMapManager::MLT_persist:
		return m_mapLocationMapPersist;
	default:
		break;
	}
	static ArchiveMapLocationMap broken;
	return broken;
}

//----------------------------------------------------------------------

void PlanetObject::getMapLocationCategories     (int parentCategory, int mapLocationType, IntVector & result) const
{
	const MapLocationVector & locationList = getMapLocationListForType (mapLocationType);	
		
	size_t const numLocations = locationList.size ();
	for (size_t i = 0; i < numLocations; ++i)
	{
		const MapLocation & ml = locationList [i];
		int cat = 0;
		if (!parentCategory)
			cat = ml.m_category;
		else if (parentCategory == ml.m_category)
			cat = ml.m_subCategory;

		if (cat)
		{
			if (std::binary_search (result.begin (), result.end (), cat))
				continue;
			
			result.push_back (cat);
			std::sort (result.begin (), result.end ());
		}
	}
}
			
//----------------------------------------------------------------------

const MapLocation * PlanetObject::getMapLocation               (int mapLocationType, const NetworkId & id) const
{
	// call this to update the cache, if necessary
	IGNORE_RETURN(getMapLocationListForType (mapLocationType));

	// but perform the search in the map (rather than the cache) for performance
	const ArchiveMapLocationMap & mapLocationMap = getMapLocationMapForType (mapLocationType);
	const ArchiveMapLocationMap::const_iterator iterFind = mapLocationMap.find(id);
	if (iterFind != mapLocationMap.end())
		return &(iterFind->second);

	return 0;
}

//----------------------------------------------------------------------

void PlanetObject::checkUpdateInternalListVersion          (int mapLocationType) const
{
	const int mapVersion      = getMapLocationVersionForTypeInternal     (mapLocationType).get ();
	int & internalListVersion = getMapLocationListVersionForTypeInternal (mapLocationType);

	if (mapVersion != internalListVersion)
	{
		MapLocationVector & mapLocationList          = const_cast<PlanetObject *>(this)->getMapLocationListForTypeInternal (mapLocationType);
		const ArchiveMapLocationMap & mapLocationMap = getMapLocationMapForType  (mapLocationType);

		mapLocationList.clear ();
		mapLocationList.reserve (mapLocationMap.size ());
		for (ArchiveMapLocationMap::const_iterator it = mapLocationMap.begin (); it != mapLocationMap.end (); ++it)
			mapLocationList.push_back ((*it).second);

		internalListVersion = mapVersion;
	}
}

//----------------------------------------------------------------------

void PlanetObject::connectedCharacterLfgDataFactionalPresenceOnErase(std::string const & keyValue, std::pair<int, int> const & value)
{
	UNREF(keyValue);

	if ((value.first > 0) || (value.second > 0))
		s_factionalPresenceSuiTableNeedRebuild = true;
}

//----------------------------------------------------------------------

void PlanetObject::connectedCharacterLfgDataFactionalPresenceOnInsert(std::string const & keyValue, std::pair<int, int> const & value)
{
	UNREF(keyValue);

	if ((value.first > 0) || (value.second > 0))
		s_factionalPresenceSuiTableNeedRebuild = true;
}

//----------------------------------------------------------------------

void PlanetObject::connectedCharacterLfgDataFactionalPresenceOnSet(std::string const & keyValue, std::pair<int, int> const & oldValue, std::pair<int, int> const & newValue)
{
	UNREF(keyValue);

	// 0 -> non-zero or non-zero -> 0
	if ((oldValue.first != newValue.first) && ((oldValue.first <= 0) || (newValue.first <= 0)))
		s_factionalPresenceSuiTableNeedRebuild = true;
	else if ((oldValue.second != newValue.second) && ((oldValue.second <= 0) || (newValue.second <= 0)))
		s_factionalPresenceSuiTableNeedRebuild = true;
}

// ----------------------------------------------------------------------

void PlanetObject::unload()
{
	FATAL(true, ("Tried to unload PlanetObject %s.", getNetworkId().getValueString().c_str()));
}

//-----------------------------------------------------------------------

void PlanetObject::onLoadedFromDatabase()
{
	UniverseObject::onLoadedFromDatabase();

	// depersist collection "server first" information
	if (m_planetName.get() != "tatooine")
		return;

	// to avoid inefficient multiple calls to the CollectionServerFirstObserver
	// class, we temporarily disable it here while we depersist collection
	// "server first" information and manually create the CollectionServerFirstObserver
	// object, so that there will only be 1 call to the CollectionServerFirstObserver
	// object when it goes out of scope at the end of the function
	m_collectionServerFirst.setSourceObject(nullptr);
	CollectionServerFirstObserver observer(this, Archive::ADOO_set);

	const DynamicVariableList & objvars = getObjVars();
	std::string objvar;
	std::vector<Unicode::String> value;
	time_t claimTime;
	NetworkId claimantId;
	Unicode::String claimantName;

	std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> const & allServerFirstCollections = CollectionsDataTable::getAllServerFirstCollections();
	for (std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
	{
		objvar = OBJVAR_COLLECTION_SERVER_FIRST + "." + iter->second->name;
		if (!objvars.hasItem(objvar) || (DynamicVariable::STRING_ARRAY != objvars.getType(objvar)))
			continue;

		value.clear();
		if (!objvars.getItem(objvar, value))
			continue;

		if (value.size() != 3)
			continue;

		claimTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(value[0]).c_str()));
		claimantId = NetworkId(Unicode::wideToNarrow(value[1]));
		claimantName = value[2];

		m_collectionServerFirst.insert(std::make_pair(std::make_pair(static_cast<int32>(claimTime), iter->second->name), std::make_pair(claimantId, claimantName)));
	}

	int collectionServerFirstUpdateNumber = 0;
	if (!objvars.getItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber))
		collectionServerFirstUpdateNumber = 0;

	m_collectionServerFirstUpdateNumber = collectionServerFirstUpdateNumber;

	// restore observer
	m_collectionServerFirst.setSourceObject(this);
}

// ----------------------------------------------------------------------

/**
 * Build or update the list of resources available on this planet, as appropriate.
 * Keeps track of the minimum expiration time of the resources in the list.  If that
 * time has passed, it rebuilds the list.
 */
void PlanetObject::buildAvailableResourceList() const
{
	if (!m_builtAvailableResourceList || ServerClock::getInstance().getGameTimeSeconds() > m_availableResourceListExpirationTime)
	{
		m_availableResourceListExpirationTime = ServerClock::cms_endOfTime;
		m_availableResourceList.clear();

		ServerResourceClassObject const * const root=NON_NULL(safe_cast<ServerResourceClassObject const *>(ServerUniverse::getInstance().getResourceTreeRoot()));
		std::vector<ResourceTypeObject const *> typeList;
		root->getAllDerivedResourceTypes(typeList);

		for (std::vector<ResourceTypeObject const *>::const_iterator i=typeList.begin(); i!=typeList.end(); ++i)
		{
			NOT_NULL(*i);
			m_availableResourceListExpirationTime = std::min(m_availableResourceListExpirationTime, (*i)->getDepletedTimestamp());
			if ((*i)->hasPoolForPlanet(getNetworkId()))
			{
				m_availableResourceList.push_back(*i);
			}
		}

		m_builtAvailableResourceList = true;
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setCollectionServerFirst(const CollectionsDataTable::CollectionInfoCollection& collectionInfo, const NetworkId& claimantId, const Unicode::String& claimantName, const std::string& claimantAccountInfo)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		if (!collectionInfo.trackServerFirst)
			return;

		const DynamicVariableList & objvars = getObjVars();
		const std::string objvar = OBJVAR_COLLECTION_SERVER_FIRST + "." + collectionInfo.name;
		if (objvars.hasItem(objvar) && (DynamicVariable::STRING_ARRAY == objvars.getType(objvar)))
			return;

		const time_t timeNow = ::time(nullptr);
		char buffer[64];
		snprintf(buffer, sizeof(buffer)-1, "%ld", timeNow);
		buffer[sizeof(buffer)-1] = '\0';

		std::vector<Unicode::String> value;
		value.push_back(Unicode::narrowToWide(buffer));
		value.push_back(Unicode::narrowToWide(claimantId.getValueString()));
		value.push_back(claimantName);

		if (setObjVarItem(objvar, value))
		{
			int collectionServerFirstUpdateNumber = 0;
			if (!objvars.getItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber))
				collectionServerFirstUpdateNumber = 0;

			++collectionServerFirstUpdateNumber;
			
			IGNORE_RETURN(setObjVarItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber));
			m_collectionServerFirstUpdateNumber = collectionServerFirstUpdateNumber;
			m_collectionServerFirst.insert(std::make_pair(std::make_pair(static_cast<int32>(timeNow), collectionInfo.name), std::make_pair(claimantId, claimantName)));

			// let claimant know "server first" claim was successful
			MessageToQueue::getInstance().sendMessageToC(claimantId,
				"C++OnCollectionServerFirst",
				collectionInfo.name,
				0,
				false);

			// CS log
			LOG("CustomerService", ("CollectionServerFirst:(%s/%s/%s) granted to %s at %ld (%s) (update #%d)",
				collectionInfo.page.book.name.c_str(),
				collectionInfo.page.name.c_str(),
				collectionInfo.name.c_str(),
				claimantAccountInfo.c_str(),
				timeNow,
				CalendarTime::convertEpochToTimeStringLocal(timeNow).c_str(),
				collectionServerFirstUpdateNumber));
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setCollectionServerFirst: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > >* const message = new MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > >(std::make_pair(std::make_pair(collectionInfo.name, claimantId), std::make_pair(claimantName, claimantAccountInfo)));
		controller->appendMessage(static_cast<int>(CM_collectionRequestServerFirst), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::removeCollectionServerFirst(const CollectionsDataTable::CollectionInfoCollection& collectionInfo)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		const DynamicVariableList & objvars = getObjVars();
		const std::string objvar = OBJVAR_COLLECTION_SERVER_FIRST + "." + collectionInfo.name;
		if (!objvars.hasItem(objvar) || (DynamicVariable::STRING_ARRAY != objvars.getType(objvar)))
			return;

		std::vector<Unicode::String> value;
		if (!objvars.getItem(objvar, value))
			return;

		if (value.size() != 3)
			return;

		const time_t claimTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(value[0]).c_str()));
		const NetworkId claimantId = NetworkId(Unicode::wideToNarrow(value[1]));
		const Unicode::String claimantName = value[2];

		removeObjVarItem(objvar);

		int collectionServerFirstUpdateNumber = 0;
		if (!objvars.getItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber))
			collectionServerFirstUpdateNumber = 0;

		++collectionServerFirstUpdateNumber;

		IGNORE_RETURN(setObjVarItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber));
		m_collectionServerFirstUpdateNumber = collectionServerFirstUpdateNumber;
		IGNORE_RETURN(m_collectionServerFirst.erase(std::make_pair(std::make_pair(static_cast<int32>(claimTime), collectionInfo.name), std::make_pair(claimantId, claimantName))));

		// CS log
		LOG("CustomerService", ("CollectionServerFirst:revoked (%s/%s/%s) which was granted to (%s, %s) at %ld (%s) (update #%d)",
			collectionInfo.page.book.name.c_str(),
			collectionInfo.page.name.c_str(),
			collectionInfo.name.c_str(),
			claimantId.getValueString().c_str(),
			Unicode::wideToNarrow(claimantName).c_str(),
			claimTime,
			CalendarTime::convertEpochToTimeStringLocal(claimTime).c_str(),
			collectionServerFirstUpdateNumber));
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::removeCollectionServerFirst: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::string>* const message = new MessageQueueGenericValueType<std::string>(collectionInfo.name);
		controller->appendMessage(static_cast<int>(CM_collectionRevokeServerFirst), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::handleCMessageTo(MessageToPayload const &message)
{
	if (message.getMethod() == "C++SetCollectionServerFirstTime")
	{
		if ((m_planetName.get() == "tatooine") && !message.getPackedDataVector().empty())
		{
			std::string const params(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			Unicode::String const delimiters(Unicode::narrowToWide("|"));
			Unicode::UnicodeStringVector tokens;
			if ((Unicode::tokenize(Unicode::narrowToWide(params), tokens, &delimiters, nullptr)) && (tokens.size() == 2))
			{
				CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName(Unicode::wideToNarrow(tokens[0]));
				if (collectionInfo && collectionInfo->trackServerFirst && (collectionInfo->serverFirstClaimTime > 0))
				{
					const DynamicVariableList & objvars = getObjVars();
					const std::string objvar = OBJVAR_COLLECTION_SERVER_FIRST + "." + collectionInfo->name;
					if (objvars.hasItem(objvar) && (DynamicVariable::STRING_ARRAY == objvars.getType(objvar)))
					{
						std::vector<Unicode::String> value;
						if (objvars.getItem(objvar, value) && (value.size() == 3))
						{
							const time_t serverFirstOldTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(value[0]).c_str()));
							const time_t serverFirstNewTime = static_cast<time_t>(::atol(Unicode::wideToNarrow(tokens[1]).c_str()));

							if (serverFirstOldTime != serverFirstNewTime)
							{
								const NetworkId claimantId = NetworkId(Unicode::wideToNarrow(value[1]));
								const Unicode::String claimantName = value[2];

								char buffer[64];
								snprintf(buffer, sizeof(buffer)-1, "%ld", serverFirstNewTime);
								buffer[sizeof(buffer)-1] = '\0';

								// set the "server first" new completion time
								value[0] = Unicode::narrowToWide(buffer);

								if (setObjVarItem(objvar, value))
								{
									int collectionServerFirstUpdateNumber = 0;
									if (!objvars.getItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber))
										collectionServerFirstUpdateNumber = 0;

									++collectionServerFirstUpdateNumber;

									IGNORE_RETURN(setObjVarItem(OBJVAR_COLLECTION_SERVER_FIRST_UPDATE_NUMBER, collectionServerFirstUpdateNumber));
									m_collectionServerFirstUpdateNumber = collectionServerFirstUpdateNumber;

									IGNORE_RETURN(m_collectionServerFirst.erase(std::make_pair(std::make_pair(static_cast<int32>(serverFirstOldTime), collectionInfo->name), std::make_pair(claimantId, claimantName))));
									m_collectionServerFirst.insert(std::make_pair(std::make_pair(static_cast<int32>(serverFirstNewTime), collectionInfo->name), std::make_pair(claimantId, claimantName)));

									// CS log
									LOG("CustomerService", ("CollectionServerFirst:(%s/%s/%s) \"server first\" completion time changed from %ld (%s) to %ld (%s) (update #%d)",
										collectionInfo->page.book.name.c_str(),
										collectionInfo->page.name.c_str(),
										collectionInfo->name.c_str(),
										serverFirstOldTime,
										CalendarTime::convertEpochToTimeStringLocal(serverFirstOldTime).c_str(),
										serverFirstNewTime,
										CalendarTime::convertEpochToTimeStringLocal(serverFirstNewTime).c_str(),
										collectionServerFirstUpdateNumber));
								}
							}
						}
					}
				}
			}
		}
	}
	else if (message.getMethod() == "C++DoGcwDecay")
	{
		// start decay handling loop
		int const now = static_cast<int>(::time(nullptr));
		int timeNextGcwScoreDecay;
		if (!getObjVars().getItem("gcwScore.nextDecayTime", timeNextGcwScoreDecay) || (timeNextGcwScoreDecay <= 0))
		{
			timeNextGcwScoreDecay = getNextGcwScoreDecayTime(now);				
			if (timeNextGcwScoreDecay >= now)
			{
				setObjVarItem("gcwScore.nextDecayTime", timeNextGcwScoreDecay);
			}
		}

		if ((timeNextGcwScoreDecay <= now) && (timeNextGcwScoreDecay > 0))
		{
			// decay GCW score
			LOG("CustomerService", ("GcwScore: decaying GCW score for period ending (%s)", CalendarTime::convertEpochToTimeStringGMT(timeNextGcwScoreDecay).c_str()));

			std::vector<NetworkId> scoreObjvar(2);
			std::map<std::string, std::pair<int64, int64> > newGcwScore;
			std::set<std::string> updatedScoreCategories;
			std::map<std::string, std::pair<int64, int64> >::const_iterator iterBegin = m_gcwImperialScore.begin();
			std::map<std::string, std::pair<int64, int64> >::const_iterator iterEnd = m_gcwImperialScore.end();
			for (; iterBegin != iterEnd; ++iterBegin)
			{
				newGcwScore[iterBegin->first] = std::make_pair((iterBegin->second.first / static_cast<int64>(2)) + (iterBegin->second.second / static_cast<int64>(2)), 0);
			}

			iterBegin = newGcwScore.begin();
			iterEnd = newGcwScore.end();
			for (; iterBegin != iterEnd; ++iterBegin)
			{
				updatedScoreCategories.insert(iterBegin->first);

				if (iterBegin->second.first <= 0)
				{
					m_gcwImperialScore.erase(iterBegin->first);
					removeObjVarItem("gcwScore.Imp." + iterBegin->first);
				}
				else
				{
					m_gcwImperialScore.set(iterBegin->first, iterBegin->second);

					scoreObjvar[0] = NetworkId(iterBegin->second.first);
					scoreObjvar[1] = NetworkId(iterBegin->second.second);
					setObjVarItem("gcwScore.Imp." + iterBegin->first, scoreObjvar);
				}
			}

			newGcwScore.clear();
			iterBegin = m_gcwRebelScore.begin();
			iterEnd = m_gcwRebelScore.end();
			for (; iterBegin != iterEnd; ++iterBegin)
			{
				newGcwScore[iterBegin->first] = std::make_pair((iterBegin->second.first / static_cast<int64>(2)) + (iterBegin->second.second / static_cast<int64>(2)), 0);
			}

			iterBegin = newGcwScore.begin();
			iterEnd = newGcwScore.end();
			for (; iterBegin != iterEnd; ++iterBegin)
			{
				updatedScoreCategories.insert(iterBegin->first);

				if (iterBegin->second.first <= 0)
				{
					m_gcwRebelScore.erase(iterBegin->first);
					removeObjVarItem("gcwScore.Reb." + iterBegin->first);
				}
				else
				{
					m_gcwRebelScore.set(iterBegin->first, iterBegin->second);

					scoreObjvar[0] = NetworkId(iterBegin->second.first);
					scoreObjvar[1] = NetworkId(iterBegin->second.second);
					setObjVarItem("gcwScore.Reb." + iterBegin->first, scoreObjvar);
				}
			}

			// we need to recalculate the score percentile for score categories that change;
			// halving both the imperial and rebel score shouldn't cause the percentile to
			// change, but as the number gets smaller, integer division truncation will start
			// to cause the percentile to change; in the worse case, when one side hits 0,
			// the other side will immediately jump to 100%, and then when that side hits 0,
			// it then becomes 50/50
			if (!updatedScoreCategories.empty())
			{
				GuildObject * const go = ServerUniverse::getInstance().getMasterGuildObject();
				if (go && go->isAuthoritative())
				{
					go->updateGcwImperialScorePercentile(updatedScoreCategories);
				}
			}

			// set up next decay calculation time
			timeNextGcwScoreDecay = getNextGcwScoreDecayTime(now + 1);				
			if (timeNextGcwScoreDecay > 0)
			{
				setObjVarItem("gcwScore.nextDecayTime", timeNextGcwScoreDecay);
			}
		}

		if (timeNextGcwScoreDecay > 0)
		{
			MessageToQueue::sendMessageToC(getNetworkId(), "C++DoGcwDecay", std::string(), std::max(0, timeNextGcwScoreDecay - now) + 1, false);
		}
	}
	else if (message.getMethod() == "C++DoGcwDecayImmediate")
	{
		if ((m_planetName.get() == "tatooine") && !message.getPackedDataVector().empty())
		{
			bool recalculatePercentile = false;

			std::string const gcwScoreCategory(message.getPackedDataVector().begin(), message.getPackedDataVector().end());
			std::map<std::string, std::pair<int64, int64> >::const_iterator iterFind = m_gcwImperialScore.find(gcwScoreCategory);
			if (iterFind != m_gcwImperialScore.end())
			{
				int64 const newScore = (iterFind->second.first / static_cast<int64>(2)) + (iterFind->second.second / static_cast<int64>(2));
				if (newScore <= 0)
				{
					m_gcwImperialScore.erase(gcwScoreCategory);
					removeObjVarItem("gcwScore.Imp." + gcwScoreCategory);
				}
				else
				{
					m_gcwImperialScore.set(gcwScoreCategory, std::make_pair(newScore, 0));

					std::vector<NetworkId> scoreObjvar(2);
					scoreObjvar[0] = NetworkId(newScore);
					scoreObjvar[1] = NetworkId(static_cast<NetworkId::NetworkIdType>(0));
					setObjVarItem("gcwScore.Imp." + gcwScoreCategory, scoreObjvar);
				}

				recalculatePercentile = true;
			}

			iterFind = m_gcwRebelScore.find(gcwScoreCategory);
			if (iterFind != m_gcwRebelScore.end())
			{
				int64 const newScore = (iterFind->second.first / static_cast<int64>(2)) + (iterFind->second.second / static_cast<int64>(2));
				if (newScore <= 0)
				{
					m_gcwRebelScore.erase(gcwScoreCategory);
					removeObjVarItem("gcwScore.Reb." + gcwScoreCategory);
				}
				else
				{
					m_gcwRebelScore.set(gcwScoreCategory, std::make_pair(newScore, 0));

					std::vector<NetworkId> scoreObjvar(2);
					scoreObjvar[0] = NetworkId(newScore);
					scoreObjvar[1] = NetworkId(static_cast<NetworkId::NetworkIdType>(0));
					setObjVarItem("gcwScore.Reb." + gcwScoreCategory, scoreObjvar);
				}

				recalculatePercentile = true;
			}

			if (recalculatePercentile)
			{
				std::set<std::string> updatedScoreCategories;
				updatedScoreCategories.insert(gcwScoreCategory);

				GuildObject * const go = ServerUniverse::getInstance().getMasterGuildObject();
				if (go && go->isAuthoritative())
				{
					go->updateGcwImperialScorePercentile(updatedScoreCategories);
				}
			}
		}
	}
	else
	{
		UniverseObject::handleCMessageTo(message);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::endBaselines()
{
	UniverseObject::endBaselines();

	if (isAuthoritative() && (m_planetName.get() == "tatooine"))
	{
		// depersist GCW score
		std::vector<NetworkId> scoreObjvar;

		{
			DynamicVariableList::NestedList const gcwImperialScore(getObjVars(), "gcwScore.Imp");
			for (DynamicVariableList::NestedList::const_iterator i = gcwImperialScore.begin(); i != gcwImperialScore.end(); ++i)
			{
				if (Pvp::getGcwScoreCategory(i.getName()) && i.getValue(scoreObjvar) && (scoreObjvar.size() == 2))
					m_gcwImperialScore.set(i.getName(), std::make_pair(scoreObjvar[0].getValue(), scoreObjvar[1].getValue()));
			}
		}

		{
			DynamicVariableList::NestedList const gcwRebelScore(getObjVars(), "gcwScore.Reb");
			for (DynamicVariableList::NestedList::const_iterator i = gcwRebelScore.begin(); i != gcwRebelScore.end(); ++i)
			{
				if (Pvp::getGcwScoreCategory(i.getName()) && i.getValue(scoreObjvar) && (scoreObjvar.size() == 2))
					m_gcwRebelScore.set(i.getName(), std::make_pair(scoreObjvar[0].getValue(), scoreObjvar[1].getValue()));
			}
		}

		// start decay handling loop
		int const now = static_cast<int>(::time(nullptr));
		int timeNextGcwScoreDecay;
		if (!getObjVars().getItem("gcwScore.nextDecayTime", timeNextGcwScoreDecay) || (timeNextGcwScoreDecay <= 0))
		{
			timeNextGcwScoreDecay = getNextGcwScoreDecayTime(now);				
			if (timeNextGcwScoreDecay >= now)
			{
				setObjVarItem("gcwScore.nextDecayTime", timeNextGcwScoreDecay);
			}
		}

		if (timeNextGcwScoreDecay > 0)
		{
			MessageToQueue::sendMessageToC(getNetworkId(), "C++DoGcwDecay", std::string(), std::max(0, timeNextGcwScoreDecay - now) + 1, false);
		}
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterLfgData(const NetworkId & characterId, const LfgCharacterData & lfgCharacterData)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if (iterFind != m_connectedCharacterLfgData.end())
		{
			// don't call decrement followed by increment if it will result in the same
			// final value, as that causes an unnecessary change to the AutoDeltaMap,
			// causing the AutoDeltaMap change handlers to unnecessarily fire
			if (iterFind->second.locationFactionalPresenceGcwRegion != lfgCharacterData.locationFactionalPresenceGcwRegion)
			{
				decrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, iterFind->second);
				incrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, lfgCharacterData);

				decrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, iterFind->second);
				incrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, lfgCharacterData);
			}
			else if ((iterFind->second.faction != lfgCharacterData.faction) && !iterFind->second.locationFactionalPresenceGcwRegion.empty())
			{
				decrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, iterFind->second);
				incrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, lfgCharacterData);

				decrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, iterFind->second);
				incrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, lfgCharacterData);
			}
			else if (((iterFind->second.locationFactionalPresenceGridX != lfgCharacterData.locationFactionalPresenceGridX) || (iterFind->second.locationFactionalPresenceGridZ != lfgCharacterData.locationFactionalPresenceGridZ)) && !iterFind->second.locationFactionalPresenceGcwRegion.empty())
			{
				decrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, iterFind->second);
				incrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, lfgCharacterData);
			}
		}
		else
		{
			incrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, lfgCharacterData);
			incrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, lfgCharacterData);
		}

		m_connectedCharacterLfgData.set(characterId, lfgCharacterData);
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterLfgData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> >(std::make_pair(characterId, lfgCharacterData));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterLfgData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

ScriptParams const *PlanetObject::getConnectedCharacterLfgDataFactionalPresenceTableDictionary()
{
	if (s_factionalPresenceSuiTableNeedRebuild)
	{
		delete s_factionalPresenceSuiTable;
		s_factionalPresenceSuiTable = nullptr;

		PlanetObject const * const tatooine = ServerUniverse::getInstance().getTatooinePlanet();
		if (tatooine)
		{
			std::map<std::string, std::pair<int, int> > const & connectedCharacterLfgDataFactionalPresence = tatooine->getConnectedCharacterLfgDataFactionalPresence();
			if (!connectedCharacterLfgDataFactionalPresence.empty())
			{
				size_t const size = connectedCharacterLfgDataFactionalPresence.size();
				s_factionalPresenceSuiTable = new ScriptParams();
				std::vector<const char *> * scriptParamsGcwContestedRegion = new std::vector<const char *>(size);
				std::vector<const char *> * scriptParamsImperial = new std::vector<const char *>(size);
				std::vector<const char *> * scriptParamsRebel = new std::vector<const char *>(size);

				char buffer[128];
				int index = 0;
				for (std::map<std::string, std::pair<int, int> >::const_iterator iter = connectedCharacterLfgDataFactionalPresence.begin(); iter != connectedCharacterLfgDataFactionalPresence.end(); ++iter, ++index)
				{
					snprintf(buffer, sizeof(buffer)-1, "@gcw_regions:%s", iter->first.c_str());
					buffer[sizeof(buffer)-1] = '\0';

					(*scriptParamsGcwContestedRegion)[index] = makeCopyOfString(buffer);
					(*scriptParamsImperial)[index] = makeCopyOfString((iter->second.first > 0) ? "X" : "");
					(*scriptParamsRebel)[index] = makeCopyOfString((iter->second.second > 0) ? "X" : "");
				}

				// column header
				static const char * s_scriptParamsColumnHeadersText[3] = 
				{
					"Region",
					"Rebel",
					"Imperial"
				};
				static std::vector<const char *> s_scriptParamsColumnHeaders(s_scriptParamsColumnHeadersText, s_scriptParamsColumnHeadersText + (sizeof(s_scriptParamsColumnHeadersText) / sizeof(const char *)));

				// column type
				static const char * s_scriptParamsColumnTypeText[3] = 
				{
					"text",
					"text",
					"text"
				};
				static std::vector<const char *> s_scriptParamsColumnType(s_scriptParamsColumnTypeText, s_scriptParamsColumnTypeText + (sizeof(s_scriptParamsColumnTypeText) / sizeof(const char *)));

				s_factionalPresenceSuiTable->addParam(s_scriptParamsColumnHeaders, "column", false);
				s_factionalPresenceSuiTable->addParam(s_scriptParamsColumnType, "columnType", false);
				s_factionalPresenceSuiTable->addParam(*scriptParamsGcwContestedRegion, "column0", true);
				s_factionalPresenceSuiTable->addParam(*scriptParamsRebel, "column1", true);
				s_factionalPresenceSuiTable->addParam(*scriptParamsImperial, "column2", true);
			}
		}

		s_factionalPresenceSuiTableNeedRebuild = false;
	}

	return s_factionalPresenceSuiTable;
}

// ----------------------------------------------------------------------

void  PlanetObject::setConnectedCharacterGroupData(const NetworkId & characterId, const NetworkId & groupId)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.groupId != groupId))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.groupId = groupId;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterGroupData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >(std::make_pair(characterId, groupId));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterGroupData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterPlanetData(const NetworkId & characterId, const std::string & planetName)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.locationPlanet != planetName))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;

			decrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, lfgCharacterData);
			decrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, lfgCharacterData);

			lfgCharacterData.locationPlanet = planetName;
			
			// changing planet, clear region and player city info, which will get updated separately
			lfgCharacterData.locationRegion.clear();
			lfgCharacterData.locationFactionalPresenceGcwRegion.clear();
			lfgCharacterData.locationFactionalPresenceGridX = 0;
			lfgCharacterData.locationFactionalPresenceGridZ = 0;
			lfgCharacterData.locationPlayerCity.clear();

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterPlanetData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<std::string, NetworkId> >* const message = new MessageQueueGenericValueType<std::pair<std::string, NetworkId> >(std::make_pair(planetName, characterId));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterPlanetData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterRegionData(const NetworkId & characterId, const std::string & regionName)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.locationRegion != regionName))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.locationRegion = regionName;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterRegionData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<std::string, NetworkId> >* const message = new MessageQueueGenericValueType<std::pair<std::string, NetworkId> >(std::make_pair(regionName, characterId));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterRegionData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterPlayerCityData(const NetworkId & characterId, const std::string & playerCityName)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.locationPlayerCity != playerCityName))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.locationPlayerCity = playerCityName;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterPlayerCityData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<std::string, NetworkId> >* const message = new MessageQueueGenericValueType<std::pair<std::string, NetworkId> >(std::make_pair(playerCityName, characterId));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterPlayerCityData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterLevelData(const NetworkId & characterId, int16 level)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.level != level))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.level = level;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterLevelData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, int> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(characterId, static_cast<int>(level)));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterLevelData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterProfessionData(const NetworkId & characterId, LfgCharacterData::Profession profession)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.profession != profession))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.profession = profession;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterProfessionData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, int> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(characterId, static_cast<int>(profession)));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterProfessionData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterSearchableByCtsSourceGalaxyData(const NetworkId & characterId, bool searchableByCtsSourceGalaxy)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.searchableByCtsSourceGalaxy != searchableByCtsSourceGalaxy))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.searchableByCtsSourceGalaxy = searchableByCtsSourceGalaxy;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterSearchableByCtsSourceGalaxyData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, int> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(characterId, static_cast<int>(searchableByCtsSourceGalaxy ? 1 : 0)));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterSearchableByCtsSourceGalaxyData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterDisplayLocationInSearchResultsData(const NetworkId & characterId, bool displayLocationInSearchResults)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.displayLocationInSearchResults != displayLocationInSearchResults))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.displayLocationInSearchResults = displayLocationInSearchResults;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterDisplayLocationInSearchResultsData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, int> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(characterId, static_cast<int>(displayLocationInSearchResults ? 1 : 0)));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterDisplayLocationInSearchResultsData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterAnonymousData(const NetworkId & characterId, bool anonymous)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.anonymous != anonymous))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.anonymous = anonymous;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterAnonymousData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, int> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, int> >(std::make_pair(characterId, static_cast<int>(anonymous ? 1 : 0)));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterAnonymousData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterInterestsData(const NetworkId & characterId, BitArray const & interests)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if ((iterFind != m_connectedCharacterLfgData.end()) && (iterFind->second.characterInterests != interests))
		{
			LfgCharacterData lfgCharacterData = iterFind->second;
			lfgCharacterData.characterInterests = interests;

			m_connectedCharacterLfgData.set(characterId, lfgCharacterData);	
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterInterestsData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<NetworkId, BitArray> >* const message = new MessageQueueGenericValueType<std::pair<NetworkId, BitArray> >(std::make_pair(characterId, interests));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterInterestsData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::setConnectedCharacterBiographyData(const NetworkId & characterId, const Unicode::String & biography)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		m_connectedCharacterBiographyData.set(characterId, biography);
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::setConnectedCharacterBiographyData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> >* const message = new MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> >(std::make_pair(biography, characterId));
		controller->appendMessage(static_cast<int>(CM_setConnectedCharacterBiographyData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::removeConnectedCharacterBiographyData(const NetworkId & characterId)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		IGNORE_RETURN(m_connectedCharacterBiographyData.erase(characterId));
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::removeConnectedCharacterBiographyData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<NetworkId>* const message = new MessageQueueGenericValueType<NetworkId>(characterId);
		controller->appendMessage(static_cast<int>(CM_removeConnectedCharacterBiographyData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

void PlanetObject::removeConnectedCharacterData(const NetworkId & characterId)
{
	if (m_planetName.get() != "tatooine")
		return;

	if (isAuthoritative())
	{
		std::map<NetworkId, LfgCharacterData>::const_iterator iterFind = m_connectedCharacterLfgData.find(characterId);
		if (iterFind != m_connectedCharacterLfgData.end())
		{
			decrementConnectedCharacterLfgDataFactionalPresenceCount(m_connectedCharacterLfgDataFactionalPresence, iterFind->second);
			decrementConnectedCharacterLfgDataFactionalPresenceGridCount(m_connectedCharacterLfgDataFactionalPresenceGrid, iterFind->second);
			IGNORE_RETURN(m_connectedCharacterLfgData.erase(characterId));
		}
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::removeConnectedCharacterData: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<NetworkId>* const message = new MessageQueueGenericValueType<NetworkId>(characterId);
		controller->appendMessage(static_cast<int>(CM_removeConnectedCharacterData), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

PlanetObject::CollectionServerFirstObserver::CollectionServerFirstObserver(PlanetObject *planet, Archive::AutoDeltaObserverOp)
: m_target(planet)
{
}

// ----------------------------------------------------------------------

PlanetObject::CollectionServerFirstObserver::~CollectionServerFirstObserver()
{
	if (m_target && (m_target->m_planetName.get() == "tatooine"))
	{
		CollectionsDataTable::setServerFirstData(m_target->m_collectionServerFirst.get());
	}
}

// ----------------------------------------------------------------------

void PlanetObject::addUniverseEvent(std::string const & eventName)
{
	if(m_planetName.get() != "tatooine")
		return;

	if(isAuthoritative())
	{
		std::string current = m_currentEvents.get();
		std::vector<std::string> currentEventVector;

		ParseEventString(current, currentEventVector);

		std::vector<std::string>::size_type i = 0;
		for(; i < currentEventVector.size(); ++i)
		{
			if(currentEventVector[i] == eventName)
			{
				return;
			}
		}

		if(current.empty())
			current.append(eventName);
		else
		{
			current.append(",");
			current.append(eventName);
		}


		m_currentEvents.set(current);
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::addUniverseEvent: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::string>* const message = new MessageQueueGenericValueType<std::string>(eventName);
		controller->appendMessage(static_cast<int>(CM_addUniverseEvent), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}

}

// ----------------------------------------------------------------------

void PlanetObject::removeUniverseEvent(std::string const & eventName)
{
	if(m_planetName.get() != "tatooine")
		return;

	if(isAuthoritative())
	{
		std::string current = m_currentEvents.get();
		std::string::size_type index = current.find(eventName);

		if( index == std::string::npos)
			return; // Event currently isn't running.

		// Sanity check because of how String find works.
		//////////////////////////////////////////////////
		std::vector<StringIndexPair> currentEventVector;
		bool entryValidated = false;
		ParseEventString(current, currentEventVector);
		
		std::vector<StringIndexPair>::size_type i = 0;
		for(; i < currentEventVector.size(); ++i)
		{
			if(currentEventVector[i].first == eventName)
			{
				index = currentEventVector[i].second;
				entryValidated = true;
			}
		}

		if(!entryValidated)
			return;
		//////////////////////////////////////////////////

		if(index != 0) // See if we need to clean up the prepending ','.
			current = current.erase(index - 1, eventName.size() + 1);
		else
			current = current.erase(index, index + eventName.size());

		m_currentEvents.set(current);
	}
	else
	{
		Controller* const controller = getController();
		if (!controller)
		{
			DEBUG_WARNING(true, ("PlanetObject::removeUniverseEvent: planet object %s has no controller", getName().c_str()));
			return;
		}

		MessageQueueGenericValueType<std::string>* const message = new MessageQueueGenericValueType<std::string>(eventName);
		controller->appendMessage(static_cast<int>(CM_removeUniverseEvent), 0.f, message, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_SERVER);
	}
}

// ----------------------------------------------------------------------

std::string const & PlanetObject::getCurrentEvents() const
{
	return m_currentEvents.get();
}

// ----------------------------------------------------------------------

void PlanetObject::EventsUpdatedCallback::modified(PlanetObject &/*target*/, std::string oldValue, std::string value, bool /*isLocal*/)
{
	if(oldValue != value)
	{
		std::vector<std::string> oldList;
		std::vector<std::string> newList;

		ParseEventString(oldValue, oldList);
		ParseEventString(value, newList);

		// First lets find out what events stopped. - O(n) UGHHHhhhhhh
		std::vector<std::string>::size_type i = 0;
		std::vector<std::string>::size_type oldListSize = oldList.size();
		std::vector<std::string>::size_type newListSize = newList.size();

		for(; i < oldListSize; ++i)
		{
			bool found = false;
			std::vector<std::string>::size_type j = 0;
			for(; j < newListSize; ++j)
			{
				if(newList[j] == oldList[i])
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				DEBUG_WARNING(ms_printEventDebug, ("------- PlanetObject - Trying to stop Event %s -------\n", oldList[i].c_str()));
				ServerBuildoutManager::onEventStopped(oldList[i]);
			}
		}

		// Now let's find out what events have started. - Same as above. Perhaps change this to a map/set/something else?
		i = 0;
		for(; i < newListSize; ++i)
		{
			bool found = false;
			std::vector<std::string>::size_type j = 0;
			for(; j < oldListSize; ++j)
			{
				if(oldList[j] == newList[i])
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				DEBUG_WARNING(ms_printEventDebug, ("------- PlanetObject - Trying to start Event %s -------\n", newList[i].c_str()));
				ServerBuildoutManager::onEventStarted(newList[i]);
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlanetObject::parseCurrentEventsList(std::vector<std::string> & outVector)
{
	std::string const events = getCurrentEvents();
	ParseEventString(events, outVector);
}

// ----------------------------------------------------------------------

void PlanetObject::updateGcwTrackingData()
{
	// send our GCW score to the other clusters and process GCW scores we received from other clusters
	if (isAuthoritative() && (ConfigServerGame::getBroadcastGcwScoreToOtherGalaxies() || ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies()))
	{
		static time_t timeNextBroadcast = ::time(nullptr) + ConfigServerGame::getBroadcastGcwScoreToOtherGalaxiesIntervalSeconds();
		time_t const timeNow = ::time(nullptr);
		if (timeNextBroadcast < timeNow)
		{
			if (ConfigServerGame::getBroadcastGcwScoreToOtherGalaxies())
			{
				std::string const & clusterName = GameServer::getInstance().getClusterName();
				GuildObject const * guildObject = ServerUniverse::getInstance().getMasterGuildObject();
				if (!clusterName.empty() && guildObject && guildObject->isAuthoritative())
				{
#ifdef _DEBUG
					// to allow testing without having to have another cluster
					std::string gcwScoreReportDifferentClusterName;
					getObjVars().getItem("gcwScoreReportDifferentClusterName", gcwScoreReportDifferentClusterName);

					const GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, std::pair<int64, int64> >, std::map<std::string, std::pair<int64, int64> > > > > gcwScoreStatRaw("GcwScoreStatRaw", std::make_pair((gcwScoreReportDifferentClusterName.empty() ? clusterName : gcwScoreReportDifferentClusterName), std::make_pair(getGcwImperialScore(), getGcwRebelScore())));
					const GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int> > > > gcwScoreStatPct("GcwScoreStatPct", std::make_pair((gcwScoreReportDifferentClusterName.empty() ? clusterName : gcwScoreReportDifferentClusterName), std::make_pair(guildObject->getGcwImperialScorePercentile(), guildObject->getGcwGroupImperialScorePercentile())));
#else
					const GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, std::pair<int64, int64> >, std::map<std::string, std::pair<int64, int64> > > > > gcwScoreStatRaw("GcwScoreStatRaw", std::make_pair(clusterName, std::make_pair(getGcwImperialScore(), getGcwRebelScore())));
					const GenericValueTypeMessage<std::pair<std::string, std::pair<std::map<std::string, int>, std::map<std::string, int> > > > gcwScoreStatPct("GcwScoreStatPct", std::make_pair(clusterName, std::make_pair(guildObject->getGcwImperialScorePercentile(), guildObject->getGcwGroupImperialScorePercentile())));
#endif

					GameServer::getInstance().sendToCentralServer(gcwScoreStatRaw);
					GameServer::getInstance().sendToCentralServer(gcwScoreStatPct);
				}
			}

			if (ConfigServerGame::getReceiveGcwScoreFromOtherGalaxies())
			{
				GuildObject * guildObject = ServerUniverse::getInstance().getMasterGuildObject();
				if (guildObject && guildObject->isAuthoritative())
				{
					guildObject->calculateGcwImperialScorePercentileForSwg();
				}
			}

			timeNextBroadcast = timeNow + ConfigServerGame::getBroadcastGcwScoreToOtherGalaxiesIntervalSeconds();
		}
	}

	if (m_nextGcwTrackingUpdate == 0)
		return;

	if (ServerClock::getInstance().getGameTimeSeconds() <= m_nextGcwTrackingUpdate)
		return;

	if (!isAuthoritative())
	{
		// send updates to auth game server
		Controller *controller = getController();
		if (controller)
		{
			if (!m_gcwImperialScoreAdjustment.empty())
			{
				controller->appendMessage(
					CM_adjustGcwImperialScore,
					0.0f,
					new MessageQueueGenericValueType<std::map<std::string, int64> >(m_gcwImperialScoreAdjustment),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}

			if (!m_gcwRebelScoreAdjustment.empty())
			{
				controller->appendMessage(
					CM_adjustGcwRebelScore,
					0.0f,
					new MessageQueueGenericValueType<std::map<std::string, int64> >(m_gcwRebelScoreAdjustment),
					GameControllerMessageFlags::SEND |
					GameControllerMessageFlags::RELIABLE |
					GameControllerMessageFlags::DEST_AUTH_SERVER);
			}
		}
	}
	else
	{
		static std::pair<int64, int64> score;
		static std::vector<NetworkId> scoreObjvar(2);

		// we'll need to recalculate the score percentile for score categories that change
		std::set<std::string> updatedScoreCategories;

		std::map<std::string, std::pair<int64, int64> >::const_iterator iterFind;

		{
			for (std::map<std::string, int64>::const_iterator iterImp = m_gcwImperialScoreAdjustment.begin(); iterImp != m_gcwImperialScoreAdjustment.end(); ++iterImp)
			{
				if (iterImp->second == 0ll)
					continue;

				iterFind = m_gcwImperialScore.find(iterImp->first);
				if (iterFind != m_gcwImperialScore.end())
				{
					score.first = iterFind->second.first;
					score.second = iterFind->second.second + (iterImp->second * static_cast<int64>(Pvp::c_gcwScoreScaleFactor));
				}
				else
				{
					score.first = 0ll;
					score.second = iterImp->second * static_cast<int64>(Pvp::c_gcwScoreScaleFactor);
				}

				// handle negative score
				if (score.second < 0ll)
				{
					score.second = 0ll;
				}

				m_gcwImperialScore.set(iterImp->first, score);

				scoreObjvar[0] = NetworkId(score.first);
				scoreObjvar[1] = NetworkId(score.second);
				setObjVarItem("gcwScore.Imp." + iterImp->first, scoreObjvar);

				updatedScoreCategories.insert(iterImp->first);
			}
		}

		{
			for (std::map<std::string, int64>::const_iterator iterReb = m_gcwRebelScoreAdjustment.begin(); iterReb != m_gcwRebelScoreAdjustment.end(); ++iterReb)
			{
				if (iterReb->second == 0ll)
					continue;

				iterFind = m_gcwRebelScore.find(iterReb->first);
				if (iterFind != m_gcwRebelScore.end())
				{
					score.first = iterFind->second.first;
					score.second = iterFind->second.second + (iterReb->second * static_cast<int64>(Pvp::c_gcwScoreScaleFactor));
				}
				else
				{
					score.first = 0ll;
					score.second = iterReb->second * static_cast<int64>(Pvp::c_gcwScoreScaleFactor);
				}

				// handle negative score
				if (score.second < 0ll)
				{
					score.second = 0ll;
				}

				m_gcwRebelScore.set(iterReb->first, score);

				scoreObjvar[0] = NetworkId(score.first);
				scoreObjvar[1] = NetworkId(score.second);
				setObjVarItem("gcwScore.Reb." + iterReb->first, scoreObjvar);

				updatedScoreCategories.insert(iterReb->first);
			}
		}

		// we need to recalculate the score percentile for score categories that change
		if (!updatedScoreCategories.empty())
		{
			GuildObject * const go = ServerUniverse::getInstance().getMasterGuildObject();
			if (go && go->isAuthoritative())
			{
				go->updateGcwImperialScorePercentile(updatedScoreCategories);
			}
		}
	}

	m_gcwImperialScoreAdjustment.clear();
	m_gcwRebelScoreAdjustment.clear();
	m_nextGcwTrackingUpdate = 0;
}

// ----------------------------------------------------------------------

void PlanetObject::adjustGcwImperialScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int64 adjustment)
{
	Pvp::GcwScoreCategory const * const gcwCategoryData = Pvp::getGcwScoreCategory(gcwCategory);
	if (!gcwCategoryData)
		return;

	// queue up adjustments and periodically update the data
	if (source != "CM_adjustGcwImperialScore")
	{
		if (sourceObject)
		{
			// if object is player object, store GCW contribution tracking information
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(sourceObject);
			if (playerObject)
			{
				int const timeNow = static_cast<int>(::time(nullptr));
				IGNORE_RETURN(playerObject->setObjVarItem(std::string("gcwContributionTracking.") + gcwCategory, timeNow));
				IGNORE_RETURN(playerObject->setObjVarItem(std::string("gcwContributionTrackingLastUpdated"), timeNow));

				// grant GCW Region Defender bonus
				float bonus = 0.0f;
				if (gcwCategoryData->gcwRegionDefender && PvpData::isImperialFactionId(sourceObject->getPvpFaction()) && (adjustment > 0) && (playerObject->getCurrentGcwRegion() == gcwCategory) && Pvp::getGcwDefenderRegionBonus(*sourceObject, *playerObject, bonus) && (bonus > 0.0f))
					adjustment += std::max(1ll, static_cast<int64>(static_cast<double>(bonus) * static_cast<double>(adjustment) / static_cast<double>(100)));
			}

			LOG("CustomerService", ("GcwScore: imperial %s %d (from %s - %s)", gcwCategory.c_str(), static_cast<int>(adjustment), source.c_str(), sourceObject->getNetworkId().getValueString().c_str()));
		}
		else
		{
			LOG("CustomerService", ("GcwScore: imperial %s %d (from %s)", gcwCategory.c_str(), static_cast<int>(adjustment), source.c_str()));
		}
	}

	std::pair<std::map<std::string, int64>::iterator, bool> result = m_gcwImperialScoreAdjustment.insert(std::make_pair(gcwCategory, adjustment));
	if (!result.second)
		result.first->second += adjustment;

	// non-auth game servers send their updates to auth game server in/every 10 seconds;
	// auth game server updates the universe data in/every 60 seconds
	if (m_nextGcwTrackingUpdate == 0)
		m_nextGcwTrackingUpdate = ServerClock::getInstance().getGameTimeSeconds() + (isAuthoritative() ? 60 : 10);
}

// ----------------------------------------------------------------------

void PlanetObject::adjustGcwRebelScore(std::string const & source, CreatureObject * sourceObject, std::string const & gcwCategory, int64 adjustment)
{
	Pvp::GcwScoreCategory const * const gcwCategoryData = Pvp::getGcwScoreCategory(gcwCategory);
	if (!gcwCategoryData)
		return;

	// queue up adjustments and periodically update the data
	if (source != "CM_adjustGcwRebelScore")
	{
		if (sourceObject)
		{
			// if object is player object, store GCW contribution tracking information
			PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(sourceObject);
			if (playerObject)
			{
				int const timeNow = static_cast<int>(::time(nullptr));
				IGNORE_RETURN(playerObject->setObjVarItem(std::string("gcwContributionTracking.") + gcwCategory, timeNow));
				IGNORE_RETURN(playerObject->setObjVarItem(std::string("gcwContributionTrackingLastUpdated"), timeNow));

				// grant GCW Region Defender bonus
				float bonus = 0.0f;
				if (gcwCategoryData->gcwRegionDefender && PvpData::isRebelFactionId(sourceObject->getPvpFaction()) && (adjustment > 0) && (playerObject->getCurrentGcwRegion() == gcwCategory) && Pvp::getGcwDefenderRegionBonus(*sourceObject, *playerObject, bonus) && (bonus > 0.0f))
					adjustment += std::max(1ll, static_cast<int64>(static_cast<double>(bonus) * static_cast<double>(adjustment) / static_cast<double>(100)));
			}

			LOG("CustomerService", ("GcwScore: rebel %s %d (from %s - %s)", gcwCategory.c_str(), static_cast<int>(adjustment), source.c_str(), sourceObject->getNetworkId().getValueString().c_str()));
		}
		else
		{
			LOG("CustomerService", ("GcwScore: rebel %s %d (from %s)", gcwCategory.c_str(), static_cast<int>(adjustment), source.c_str()));
		}
	}

	std::pair<std::map<std::string, int64>::iterator, bool> result = m_gcwRebelScoreAdjustment.insert(std::make_pair(gcwCategory, adjustment));
	if (!result.second)
		result.first->second += adjustment;

	// non-auth game servers send their updates to auth game server in/every 10 seconds;
	// auth game server updates the universe data in/every 60 seconds
	if (m_nextGcwTrackingUpdate == 0)
		m_nextGcwTrackingUpdate = ServerClock::getInstance().getGameTimeSeconds() + (isAuthoritative() ? 60 : 10);
}

// ----------------------------------------------------------------------
// the rebel score is 100 - the imperial score
int PlanetObject::getGcwImperialScorePercentile(std::string const & gcwCategory) const
{
	return Pvp::calculateGcwImperialScorePercentile(gcwCategory, m_gcwImperialScore.getMap(), m_gcwRebelScore.getMap());
}

// ----------------------------------------------------------------------

int PlanetObjectNamespace::getNextGcwScoreDecayTime(int base)
{
	int nextTime = -1;
	int time;

	std::vector<std::pair<std::pair<int, int>, std::pair<int, int> > > const & gcwScoreDecayTime = ConfigServerGame::getGcwScoreDecayTime();
	for (std::vector<std::pair<std::pair<int, int>, std::pair<int, int> > >::const_iterator iter = gcwScoreDecayTime.begin(); iter != gcwScoreDecayTime.end(); ++iter)
	{
		time = static_cast<int>(CalendarTime::getNextGMTTimeOcurrence(static_cast<time_t>(base), iter->first.first, iter->first.second, iter->second.first, iter->second.second));
		if (time > 0)
		{
			if ((nextTime < 0) || (time < nextTime))
				nextTime = time;
		}
	}

	if (nextTime < 0)
		nextTime = static_cast<int>(::time(nullptr)) + (60 * 60 * 24 * 7);

	return nextTime;
}

// ----------------------------------------------------------------------

void PlanetObjectNamespace::decrementConnectedCharacterLfgDataFactionalPresenceCount(Archive::AutoDeltaMap<std::string, std::pair<int, int>, PlanetObject> & connectedCharacterLfgDataFactionalPresence, const LfgCharacterData & lfgCharacterData)
{
	if (!lfgCharacterData.locationFactionalPresenceGcwRegion.empty())
	{
		std::map<std::string, std::pair<int, int> >::const_iterator iterFindFactionPresence = connectedCharacterLfgDataFactionalPresence.find(lfgCharacterData.locationFactionalPresenceGcwRegion);
		if (iterFindFactionPresence != connectedCharacterLfgDataFactionalPresence.end())
		{
			std::pair<int, int> count = iterFindFactionPresence->second;
			if (PvpData::isImperialFactionId(lfgCharacterData.faction))
				count.first = std::max(0, count.first - 1);
			else if (PvpData::isRebelFactionId(lfgCharacterData.faction))
				count.second = std::max(0, count.second - 1);

			if ((count.first <= 0) && (count.second <= 0))
				IGNORE_RETURN(connectedCharacterLfgDataFactionalPresence.erase(lfgCharacterData.locationFactionalPresenceGcwRegion));
			else
				connectedCharacterLfgDataFactionalPresence.set(lfgCharacterData.locationFactionalPresenceGcwRegion, count);
		}
	}
}

// ----------------------------------------------------------------------

void PlanetObjectNamespace::incrementConnectedCharacterLfgDataFactionalPresenceCount(Archive::AutoDeltaMap<std::string, std::pair<int, int>, PlanetObject> & connectedCharacterLfgDataFactionalPresence, const LfgCharacterData & lfgCharacterData)
{
	if (!lfgCharacterData.locationFactionalPresenceGcwRegion.empty())
	{
		std::map<std::string, std::pair<int, int> >::const_iterator iterFindFactionPresence = connectedCharacterLfgDataFactionalPresence.find(lfgCharacterData.locationFactionalPresenceGcwRegion);
		if (iterFindFactionPresence != connectedCharacterLfgDataFactionalPresence.end())
		{
			if (PvpData::isImperialFactionId(lfgCharacterData.faction))
				connectedCharacterLfgDataFactionalPresence.set(lfgCharacterData.locationFactionalPresenceGcwRegion, std::make_pair(iterFindFactionPresence->second.first + 1, iterFindFactionPresence->second.second));
			else if (PvpData::isRebelFactionId(lfgCharacterData.faction))
				connectedCharacterLfgDataFactionalPresence.set(lfgCharacterData.locationFactionalPresenceGcwRegion, std::make_pair(iterFindFactionPresence->second.first, iterFindFactionPresence->second.second + 1));
		}
		else if (PvpData::isImperialFactionId(lfgCharacterData.faction))
		{
			connectedCharacterLfgDataFactionalPresence.set(lfgCharacterData.locationFactionalPresenceGcwRegion, std::make_pair(1, 0));
		}
		else if (PvpData::isRebelFactionId(lfgCharacterData.faction))
		{
			connectedCharacterLfgDataFactionalPresence.set(lfgCharacterData.locationFactionalPresenceGcwRegion, std::make_pair(0, 1));
		}
	}
}

// ----------------------------------------------------------------------

void PlanetObjectNamespace::decrementConnectedCharacterLfgDataFactionalPresenceGridCount(Archive::AutoDeltaMap<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > > & connectedCharacterLfgDataFactionalPresenceGrid, const LfgCharacterData & lfgCharacterData)
{
	// factional presence grid tracking is only used for ground
	if (::strncmp("space_", lfgCharacterData.locationPlanet.c_str(), 6) == 0)
		return;

	if (!lfgCharacterData.locationFactionalPresenceGcwRegion.empty())
	{
		std::map<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > >::const_iterator iterFindFactionPresence = connectedCharacterLfgDataFactionalPresenceGrid.find(std::make_pair(lfgCharacterData.locationPlanet, std::make_pair(lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ)));
		if (iterFindFactionPresence != connectedCharacterLfgDataFactionalPresenceGrid.end())
		{
			std::pair<NetworkId, std::pair<int, int> > const oldValue = iterFindFactionPresence->second;
			std::pair<NetworkId, std::pair<int, int> > newValue = iterFindFactionPresence->second;
			if (PvpData::isImperialFactionId(lfgCharacterData.faction))
				newValue.second.first = std::max(0, newValue.second.first - 1);
			else if (PvpData::isRebelFactionId(lfgCharacterData.faction))
				newValue.second.second = std::max(0, newValue.second.second - 1);

			if ((newValue.second.first <= 0) && (newValue.second.second <= 0))
			{
				IGNORE_RETURN(connectedCharacterLfgDataFactionalPresenceGrid.erase(std::make_pair(lfgCharacterData.locationPlanet, std::make_pair(lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ))));

				// remove from planet map location
				if ((oldValue.second.first > 0) || (oldValue.second.second > 0))
					removeFactionalPresenceGridFromPlanetMap(lfgCharacterData.locationPlanet, oldValue.first);
			}
			else
			{
				// non-zero -> 0
				if ((oldValue.second.first != newValue.second.first) && (newValue.second.first <= 0))
				{
					removeFactionalPresenceGridFromPlanetMap(lfgCharacterData.locationPlanet, oldValue.first);

					newValue.first = ObjectIdManager::getInstance().getNewObjectId();
					addFactionalPresenceGridToPlanetMap(lfgCharacterData.locationPlanet, newValue.first, lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ, (newValue.second.first > 0), (newValue.second.second > 0));
				}
				else if ((oldValue.second.second != newValue.second.second) && (newValue.second.second <= 0))
				{
					removeFactionalPresenceGridFromPlanetMap(lfgCharacterData.locationPlanet, oldValue.first);

					newValue.first = ObjectIdManager::getInstance().getNewObjectId();
					addFactionalPresenceGridToPlanetMap(lfgCharacterData.locationPlanet, newValue.first, lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ, (newValue.second.first > 0), (newValue.second.second > 0));
				}

				connectedCharacterLfgDataFactionalPresenceGrid.set(std::make_pair(lfgCharacterData.locationPlanet, std::make_pair(lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ)), newValue);
			}
		}
	}
}

// ----------------------------------------------------------------------

void PlanetObjectNamespace::incrementConnectedCharacterLfgDataFactionalPresenceGridCount(Archive::AutoDeltaMap<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > > & connectedCharacterLfgDataFactionalPresenceGrid, const LfgCharacterData & lfgCharacterData)
{
	// factional presence grid tracking is only used for ground
	if (::strncmp("space_", lfgCharacterData.locationPlanet.c_str(), 6) == 0)
		return;

	if (!lfgCharacterData.locationFactionalPresenceGcwRegion.empty())
	{
		std::pair<NetworkId, std::pair<int, int> > oldValue;
		std::pair<NetworkId, std::pair<int, int> > newValue;

		std::map<std::pair<std::string, std::pair<int, int> >, std::pair<NetworkId, std::pair<int, int> > >::const_iterator iterFindFactionPresence = connectedCharacterLfgDataFactionalPresenceGrid.find(std::make_pair(lfgCharacterData.locationPlanet, std::make_pair(lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ)));
		if (iterFindFactionPresence != connectedCharacterLfgDataFactionalPresenceGrid.end())
		{
			oldValue = iterFindFactionPresence->second;
			newValue = iterFindFactionPresence->second;
		}
		else
		{
			oldValue = std::make_pair(NetworkId::cms_invalid, std::make_pair(0, 0));
			newValue = std::make_pair(NetworkId::cms_invalid, std::make_pair(0, 0));
		}

		if (PvpData::isImperialFactionId(lfgCharacterData.faction))
			++(newValue.second.first);
		else if (PvpData::isRebelFactionId(lfgCharacterData.faction))
			++(newValue.second.second);

		// 0 -> non-zero
		if ((oldValue.second.first != newValue.second.first) && (oldValue.second.first <= 0))
		{
			if (oldValue.first.isValid())
				removeFactionalPresenceGridFromPlanetMap(lfgCharacterData.locationPlanet, oldValue.first);

			newValue.first = ObjectIdManager::getInstance().getNewObjectId();
			addFactionalPresenceGridToPlanetMap(lfgCharacterData.locationPlanet, newValue.first, lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ, (newValue.second.first > 0), (newValue.second.second > 0));
		}
		else if ((oldValue.second.second != newValue.second.second) && (oldValue.second.second <= 0))
		{
			if (oldValue.first.isValid())
				removeFactionalPresenceGridFromPlanetMap(lfgCharacterData.locationPlanet, oldValue.first);

			newValue.first = ObjectIdManager::getInstance().getNewObjectId();
			addFactionalPresenceGridToPlanetMap(lfgCharacterData.locationPlanet, newValue.first, lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ, (newValue.second.first > 0), (newValue.second.second > 0));
		}

		connectedCharacterLfgDataFactionalPresenceGrid.set(std::make_pair(lfgCharacterData.locationPlanet, std::make_pair(lfgCharacterData.locationFactionalPresenceGridX, lfgCharacterData.locationFactionalPresenceGridZ)), newValue);
	}
}

// ----------------------------------------------------------------------

void PlanetObjectNamespace::removeFactionalPresenceGridFromPlanetMap(std::string const & planetName, NetworkId const & locationId)
{
	// factional presence grid tracking is only used for ground
	if (::strncmp("space_", planetName.c_str(), 6) == 0)
		return;

	PlanetObject * const planet = ServerUniverse::getInstance().getPlanetByName(planetName);
	if (planet)
		planet->removeMapLocation(locationId, static_cast<int>(PlanetMapManager::MLT_dynamic));
}

// ----------------------------------------------------------------------

void PlanetObjectNamespace::addFactionalPresenceGridToPlanetMap(std::string const & planetName, NetworkId const & locationId, int gridX, int gridZ, bool imperial, bool rebel)
{
	// factional presence grid tracking is only used for ground
	if (::strncmp("space_", planetName.c_str(), 6) == 0)
		return;

	if (!imperial && !rebel)
		return;

	PlanetObject * const planet = ServerUniverse::getInstance().getPlanetByName(planetName);
	if (planet)
	{
		MapLocation ml;
		ml.m_locationId = locationId;

		if (imperial && rebel)
		{
			ml.m_subCategory = PlanetMapManager::findCategory("gcw_factional_presence_rebel_and_imperial");
			ml.m_locationName = Unicode::narrowToWide(FormattedString<512>().sprintf("\\#FFFFFFGCW FP Grid (%d, %d) -> (%d, %d)", (gridX - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridZ - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridX + (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridZ + (PvpData::getGcwFactionalPresenceGridSize() / 2))));
		}
		else if (imperial)
		{
			ml.m_subCategory = PlanetMapManager::findCategory("gcw_factional_presence_imperial_only");
			ml.m_locationName = Unicode::narrowToWide(FormattedString<512>().sprintf("\\#C2B5C9GCW FP Grid (%d, %d) -> (%d, %d)", (gridX - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridZ - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridX + (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridZ + (PvpData::getGcwFactionalPresenceGridSize() / 2))));
		}
		else
		{
			ml.m_subCategory = PlanetMapManager::findCategory("gcw_factional_presence_rebel_only");
			ml.m_locationName = Unicode::narrowToWide(FormattedString<512>().sprintf("\\#D93311GCW FP Grid (%d, %d) -> (%d, %d)", (gridX - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridZ - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridX + (PvpData::getGcwFactionalPresenceGridSize() / 2)), (gridZ + (PvpData::getGcwFactionalPresenceGridSize() / 2))));
		}

		ml.m_location = Vector2d(static_cast<float>(gridX), static_cast<float>(gridZ));
		ml.m_category = PlanetMapManager::findCategory("gcw_factional_presence");

		planet->addMapLocation(ml, static_cast<int>(PlanetMapManager::MLT_dynamic), false);
	}
}

// ======================================================================
