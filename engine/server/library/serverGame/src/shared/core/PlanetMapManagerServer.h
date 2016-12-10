//======================================================================
//
// PlanetMapManagerServer.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlanetMapManagerServer_H
#define INCLUDED_PlanetMapManagerServer_H

//======================================================================

class GetMapLocationsMessage;
class Client;
class MapLocation;
class NetworkId;

//----------------------------------------------------------------------

class PlanetMapManagerServer
{
public:

	typedef std::vector<MapLocation> MapLocationVector;
	typedef std::vector<int>         IntVector;

	static void                handleClientRequest         (const Client & client, const GetMapLocationsMessage & msg);
	static void                handleClientRequestGetSpecificMapLocationsMessage (const Client & client, const std::set<std::pair<std::string, NetworkId> > & msg);

	static bool                addMapLocation              (const MapLocation & loc, int mapLocationType, bool enforceLocationCountLimits);
	static bool                removeMapLocation           (const NetworkId & id);
	static void                getMapLocationsByCategories (int mapLocationType, std::string & categoryName, std::string & subCategoryName, MapLocationVector & locs);
	static void                getMapLocationsByCategories (int mapLocationType, uint8 categoryName, uint8 subCategoryName, MapLocationVector & locs);

	static void                getCategories               (const std::string & parentCategory, IntVector & results);
	static void                getCategories               (int parentCategory, IntVector & results);
	static void                getCategories               (int parentCategory, int mapLocationType, IntVector & results);

	static const MapLocation * getLocation                 (const NetworkId & id, int & mapLocationType);
	static const MapLocation * getLocationByType           (const NetworkId & id, int mapLocationType);
};

//======================================================================

#endif


