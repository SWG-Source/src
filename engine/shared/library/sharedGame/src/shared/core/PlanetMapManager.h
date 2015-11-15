//======================================================================
//
// PlanetMapManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlanetMapManager_H
#define INCLUDED_PlanetMapManager_H

//======================================================================

class Vector;
class MapLocation;
class NetworkId;
class Vector2d;

//----------------------------------------------------------------------

class PlanetMapManager
{
public:

	enum MapLocationType
	{
		MLT_static,
		MLT_dynamic,
		MLT_persist,
		MLT_numTypes
	};

	static Vector convertPositionToFictional   (const std::string & sceneId, const Vector & pos_w);
	static Vector convertPositionFromFictional (const std::string & sceneId, const Vector & fictionalPos_w);

	static const std::string &     findCategoryName     (uint8 category);
	static const StringId &        findCategoryStringId (uint8 category);
	static uint8                   findCategory         (const std::string & name, bool warn = true);

	static uint8                   getCityCategory      ();
	static uint8                   getWaypointCategory  ();
	static uint8                   getPoiCategory       ();
	static uint8				   getGCWRegionCategory ();

	static MapLocation             makeMapLocation (const NetworkId &locationId, const Unicode::String &locationName, const Vector2d &location, const std::string &categoryName, const std::string &subCategoryName, int flags, bool & result);

	static bool                    checkCategoryFaction (uint8 category, uint32 & faction, bool & factionVisibilityOnly);

	static bool                    sceneHasSupermap(std::string const & sceneId);
	static std::string             getSceneSupermap(std::string const & sceneId);
};

//======================================================================

#endif
