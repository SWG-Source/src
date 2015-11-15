//======================================================================
//
// PlanetMapManagerServer.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlanetMapManagerServer.h"

#include "serverGame/Client.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TangibleObject.h"
#include "sharedGame/PlanetMapManager.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/GetMapLocationsMessage.h"
#include "sharedNetworkMessages/GetMapLocationsResponseMessage.h"

#include <algorithm>

//======================================================================

namespace PlanetMapManagerServerNamespace
{

}

using namespace PlanetMapManagerServerNamespace;

//----------------------------------------------------------------------

void PlanetMapManagerServer::handleClientRequest (const Client & client, const GetMapLocationsMessage & msg)
{
	const std::string & planetName = msg.getPlanetName ();

	//-- ignore requests for other planets
	if (planetName != ServerWorld::getSceneId ())
		return;

	uint32 clientFaction = 0;
	const TangibleObject * const tangible = dynamic_cast<const TangibleObject *>(client.getCharacterObject ());
	if (tangible)
		clientFaction = tangible->getPvpFaction ();

	const PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer no planet for [%s]", planetName.c_str ()));
		return;
	}

	const int cacheVersionStatic  = msg.getCacheVersionStatic  ();
	const int cacheVersionDynamic = msg.getCacheVersionDynamic ();
	const int cacheVersionPersist = msg.getCacheVersionPersist ();

	const int currentCacheVersionStatic  = planetObject->getMapLocationVersionForType (PlanetMapManager::MLT_static);
	const int currentCacheVersionDynamic = planetObject->getMapLocationVersionForType (PlanetMapManager::MLT_dynamic);
	const int currentCacheVersionPersist = planetObject->getMapLocationVersionForType (PlanetMapManager::MLT_persist);

	const bool updateStatic  = !cacheVersionStatic  || cacheVersionStatic  != currentCacheVersionStatic;
	const bool updateDynamic = !cacheVersionDynamic || cacheVersionDynamic != currentCacheVersionDynamic;
	const bool updatePersist = !cacheVersionPersist || cacheVersionPersist != currentCacheVersionPersist;

	//-- only retrieve the map location vectors for the out of date cache versions

	static const MapLocationVector locationsEmpty;

	const MapLocationVector & locationsStatic   = updateStatic ?  planetObject->getMapLocations (PlanetMapManager::MLT_static)  : locationsEmpty;
	const MapLocationVector & locationsDynamic  = updateDynamic ? planetObject->getMapLocations (PlanetMapManager::MLT_dynamic) : locationsEmpty;
	const MapLocationVector & locationsPersist  = updatePersist ? planetObject->getMapLocations (PlanetMapManager::MLT_persist) : locationsEmpty;

	static MapLocationVector filteredVectors [PlanetMapManager::MLT_numTypes];

	const MapLocationVector * mapLocationVectors [PlanetMapManager::MLT_numTypes] =
	{
		&locationsStatic,
		&locationsDynamic,
		&locationsPersist
	};

	for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
	{
		MapLocationVector & filtered  = filteredVectors [i];
		const MapLocationVector & mlv = *mapLocationVectors [i];
		filtered.reserve (mlv.size ());
		filtered.clear ();

		uint32 ml_faction = 0;
		bool   ml_faction_vis = false;

		for (MapLocationVector::const_iterator it = mlv.begin (); it != mlv.end (); ++it)
		{
			const MapLocation & ml = *it;
			const uint8 cat = ml.getCategory ();

			if (PlanetMapManager::checkCategoryFaction (cat, ml_faction, ml_faction_vis))
			{
				if (ml_faction_vis && ml_faction != clientFaction)
					continue;
			}
				
			filtered.push_back (ml);
		}
	}

	const GetMapLocationsResponseMessage rsp (planetName, filteredVectors [0], filteredVectors [1], filteredVectors [2], currentCacheVersionStatic, currentCacheVersionDynamic, currentCacheVersionPersist);
	client.send (rsp, true);
}

//----------------------------------------------------------------------


void PlanetMapManagerServer::handleClientRequestGetSpecificMapLocationsMessage (const Client & client, const std::set<std::pair<std::string, NetworkId> > & msg)
{
	std::vector<std::pair<std::string, MapLocation> > response;

	for (std::set<std::pair<std::string, NetworkId> >::const_iterator iter = msg.begin(); iter != msg.end(); ++iter)
	{
		const PlanetObject * const planetObject = ServerUniverse::getInstance().getPlanetByName(iter->first);
		if (planetObject)
		{
			for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
			{
				const MapLocation * const ml = planetObject->getMapLocation(i, iter->second);
				if (ml)
				{
					response.push_back(std::make_pair(iter->first, *ml));
					break;
				}
			}
		}
	}

	const GenericValueTypeMessage<std::vector<std::pair<std::string, MapLocation> > > rsp("GetSpecificMapLocationsResponseMessage", response);
	client.send (rsp, true);
}

//----------------------------------------------------------------------

bool PlanetMapManagerServer::addMapLocation (const MapLocation & loc, int mapLocationType, bool enforceLocationCountLimits)
{
	if (mapLocationType < 0 || mapLocationType >= PlanetMapManager::MLT_numTypes)
	{
		WARNING (true, ("PlanetMapManagerServer::addMapLocation invalid type [%d]", mapLocationType));
		return false;
	}

	const std::string & planetName = ServerWorld::getSceneId ();

	PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer::addMapLocation no planet for [%s]", planetName.c_str ()));
		return false;
	}

	return planetObject->addMapLocation(loc, mapLocationType, enforceLocationCountLimits);
}

//----------------------------------------------------------------------

bool PlanetMapManagerServer::removeMapLocation (const NetworkId & id)
{
	const std::string & planetName = ServerWorld::getSceneId ();

	PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer::removeMapLocation no planet for [%s]", planetName.c_str ()));
		return false;
	}

	return planetObject->removeMapLocation(id);
}

//----------------------------------------------------------------------

void PlanetMapManagerServer::getMapLocationsByCategories (int mapLocationType, std::string & categoryName, std::string & subCategoryName, MapLocationVector & locs)
{
	const uint8 category    = PlanetMapManager::findCategory (categoryName);
	const uint8 subCategory = PlanetMapManager::findCategory (subCategoryName);

	getMapLocationsByCategories (mapLocationType, category, subCategory, locs);
}

//----------------------------------------------------------------------

void PlanetMapManagerServer::getMapLocationsByCategories (int mapLocationType, uint8 category, uint8 subCategory, MapLocationVector & locs)
{
	const std::string & planetName = ServerWorld::getSceneId ();

	const PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer::getMapLocationsByCategories no planet for [%s]", planetName.c_str ()));
		return;
	}

	planetObject->getMapLocationsByCategories (mapLocationType, category, subCategory, locs);

}

//----------------------------------------------------------------------

void PlanetMapManagerServer::getCategories               (int parentCategory, IntVector & results)
{
	static IntVector tmp_iv;
	tmp_iv.clear ();

	for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
	{
		getCategories  (parentCategory, i, tmp_iv);
		results.insert (results.end (), tmp_iv.begin (), tmp_iv.end ());
	}

	std::sort   (results.begin (), results.end ());
	std::unique (results.begin (), results.end ());
}

//----------------------------------------------------------------------

void PlanetMapManagerServer::getCategories               (int parentCategory, int mapLocationType, IntVector & results)
{
	const std::string & planetName = ServerWorld::getSceneId ();

	const PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer::getMapLocationsByCategories no planet for [%s]", planetName.c_str ()));
		return;
	}

	//-- this function returns a sorted vector
	planetObject->getMapLocationCategories (parentCategory, mapLocationType, results);
}

//----------------------------------------------------------------------

const MapLocation * PlanetMapManagerServer::getLocation                 (const NetworkId & id, int & mapLocationType)
{
	const std::string & planetName = ServerWorld::getSceneId ();

	const PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer::getMapLocationsByCategories no planet for [%s]", planetName.c_str ()));
		return 0;
	}

	for (int i = 0; i < static_cast<int>(PlanetMapManager::MLT_numTypes); ++i)
	{
		const MapLocation * const ml = planetObject->getMapLocation (i, id);
		if (ml)
		{
			mapLocationType = i;
			return ml;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

const MapLocation * PlanetMapManagerServer::getLocationByType           (const NetworkId & id, int mapLocationType)
{
	const std::string & planetName = ServerWorld::getSceneId ();

	const PlanetObject * const planetObject = ServerUniverse::getInstance ().getPlanetByName (planetName);
	if (!planetObject)
	{
		WARNING (true, ("PlanetMapManagerServer::getMapLocationsByCategories no planet for [%s]", planetName.c_str ()));
		return 0;
	}

	return planetObject->getMapLocation (mapLocationType, id);
}

//======================================================================
