//========================================================================
//
// RegionMaster.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_RegionMaster_H
#define INCLUDED_RegionMaster_H

// ======================================================================

class BattlefieldMarkerObject;
class MxCifQuadTree;
class NetworkId;
class Region;
class ServerObject;
class UniverseObject;
class Vector;

// ======================================================================

/** 
 * Keeps track of all regions in the game.
 */
class RegionMaster
{
public:
	// this enum list is so we know when to create a pvp region as opposed to a 
	// normal region when creating dynamic regions
	enum PvpRegion
	{
		PR_normal,
		PR_truce,
		PR_battlefieldPvp,
		PR_battlefieldPve
	};

public:

	typedef std::vector<const Region *> RegionVector;
	static void install();
	static void exit();

	static void createNewDynamicRegion                    (float minX, float minZ, float maxX, float maxZ, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify);
	static void createNewDynamicRegion                    (float centerX, float centerZ, float radius, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify);
	static NetworkId createNewDynamicRegionWithSpawn      (float centerX, float centerY, float centerZ, float radius, const Unicode::String & name, const std::string & planet, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify, std::string spawnDatatable, int duration);

	static const Region * addDynamicRegion                (const UniverseObject & source);
	static void           removeDynamicRegion             (const UniverseObject & source);
	static const Region * getDynamicRegionFromObject      (const UniverseObject & source);

	static Region *       getRegionByName                 (const std::string & planet, const Unicode::String & regionName);
	static const Region * getSmallestRegionAtPoint        (const std::string & planet, float x, float z);
	static const Region * getSmallestVisibleRegionAtPoint (const std::string & planet, float x, float z);
	static void           getRegionsAtPoint               (const std::string & planet, float x, float z, RegionVector & regions);

	static void           getRegionsForPlanet             (const std::string & planetName, RegionVector & regions);

	static void           registerBattlefieldMarker       (const std::string & regionName, BattlefieldMarkerObject *marker);

private:

	// disabled
	RegionMaster();
	RegionMaster(const RegionMaster& rhs);
	RegionMaster& operator=(const RegionMaster& rhs);

private:
	static void readRegionDataTables       ();
	static bool setDynamicRegionObjectData (UniverseObject & object, const Unicode::String & name, const std::string & planet, int geometry, float minX, float minZ, float maxX, float maxZ, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify);
	static bool setDynamicSpawnRegionObjectData (UniverseObject & object, const Unicode::String & name, const std::string & planet, int geometry, float minX, float minZ, float maxX, float maxZ, int pvp, int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable, int mission, bool visible, bool notify, std::string spawntable, int duration);
	// we need to get regions by position or name, so we have fast lookup
	// structures for both, at the expense of using more memory
	typedef std::map<Unicode::String, Region *> RegionsMappedByName;
	struct RegionData;

	// regions per planet
	static std::map<std::string, RegionData> ms_planetRegions;

	// regions created from the region datatables
	static RegionVector ms_staticRegions;

	// regions created from scripts
	static RegionVector ms_dynamicRegions;

	static bool ms_installed;
};


#endif	// INCLUDED_RegionMaster_H

