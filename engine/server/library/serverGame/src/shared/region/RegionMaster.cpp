//========================================================================
//
// RegionMaster.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/RegionMaster.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionPvp.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/UniverseObject.h"
#include "serverNetworkMessages/CreateDynamicRegionCircleMessage.h"
#include "serverNetworkMessages/CreateDynamicRegionRectangleMessage.h"
#include "serverNetworkMessages/CreateDynamicSpawnRegionCircleMessage.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/FloatMath.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/MxCifQuadTree.h"
#include "sharedMath/MxCifQuadTreeBounds.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <unordered_set>
#include <map>
#include <vector>


// objvars for dynamic regions
const static std::string OBJVAR_DYNAMIC_REGION("dynamic_region");
const static std::string OBJVAR_DYNAMIC_REGION_PLANET("planet");
const static std::string OBJVAR_DYNAMIC_REGION_NAME("name");
const static std::string OBJVAR_DYNAMIC_REGION_GEOMETRY("geometry");
const static std::string OBJVAR_DYNAMIC_REGION_MINX("minX");
const static std::string OBJVAR_DYNAMIC_REGION_MINY("minY");
const static std::string OBJVAR_DYNAMIC_REGION_MAXX("maxX");
const static std::string OBJVAR_DYNAMIC_REGION_MAXY("maxY");
const static std::string OBJVAR_DYNAMIC_REGION_PVP("pvp");
const static std::string OBJVAR_DYNAMIC_REGION_GEOGRAPHY("geography");
const static std::string OBJVAR_DYNAMIC_REGION_MIN_DIFFICULTY("minDifficulty");
const static std::string OBJVAR_DYNAMIC_REGION_MAX_DIFFICULTY("maxDifficulty");
const static std::string OBJVAR_DYNAMIC_REGION_SPAWN("spawn");
const static std::string OBJVAR_DYNAMIC_REGION_MISSION("mission");
const static std::string OBJVAR_DYNAMIC_REGION_BUILDABLE("buildable");
const static std::string OBJVAR_DYNAMIC_REGION_MUNICIPAL("municipal");
const static std::string OBJVAR_DYNAMIC_REGION_VISIBLE("visible");
const static std::string OBJVAR_DYNAMIC_REGION_BATTLEFIELD_MARKER("_battlefield");
const static std::string OBJVAR_DYNAMIC_REGION_NOTIFY("notify");
const static std::string OBJVAR_DYNAMIC_REGION_SPAWNTABLE("spawntable");
const static std::string OBJVAR_DYNAMIC_REGION_DURATION("duration");
const static std::string OBJVAR_DYNAMIC_REGION_MAXDURATION("maxDuration");
const static std::string OBJVAR_DYNAMIC_REGION_BIRTH("birth");
const static std::string OBJVAR_DYNAMIC_REGION_READABLE_BIRTH("calendarBirth");
const static std::string OBJVAR_DYNAMIC_REGION_READABLE_DEATH("calenderDeath");
const static std::string OBJVAR_DYNAMIC_REGION_CREATURE_LIST("creatureList");

// template used to make dynamic regions
const static std::string DYNAMIC_REGION_TEMPLATE_NAME("object/universe/dynamic_region.iff");



enum RegionGeometry
{
	RG_rectagle,
	RG_circle
};

//----------------------------------------------------------------------

struct RegionMaster::RegionData
{
	MxCifQuadTree *     tree;
	RegionsMappedByName nameMap;
	
	RegionData() : tree(nullptr), nameMap() {}
};

namespace RegionMasterNamspace
{
	EnvironmentInfo convertFromStringToEnvironmentInfo(std::string const & stringList);

	std::string                            s_environmentFlagsDataTable  = "datatables/region/region_environment_flags.iff";
	std::map<std::string, EnvironmentInfo> s_conversionMap;

	std::string							   s_dynamicMasterScript = "systems.regions.dynamic_region_master";
}

using namespace RegionMasterNamspace;
//========================================================================
// static member vars

std::map<std::string, RegionMaster::RegionData> RegionMaster::ms_planetRegions;
std::vector<const Region *>                     RegionMaster::ms_staticRegions;
std::vector<const Region *>                     RegionMaster::ms_dynamicRegions;
bool                                            RegionMaster::ms_installed = false;

//========================================================================

/**
 * Called on program startup. Creates the RegionMaster.
 */
void RegionMaster::install()
{
	if (!ms_installed)
	{
		ms_installed = true;
		readRegionDataTables();
		ExitChain::add(exit, "RegionMaster");
	}
}

//-----------------------------------------------------------------------

/**
 * Called on program shutdown. Deletes the RegionMaster.
 */
void RegionMaster::exit()
{
	if (ms_installed)
	{
		{
			for (std::map<std::string, RegionData>::iterator i1 = ms_planetRegions.begin();
				i1 != ms_planetRegions.end(); ++i1)
			{
				delete (*i1).second.tree;
				(*i1).second.tree = nullptr;
			}
			ms_planetRegions.clear();
		}

		{
			for (std::vector<const Region *>::iterator i2 = ms_staticRegions.begin();
				i2 != ms_staticRegions.end(); ++i2)
			{
				delete const_cast<Region *>(*i2);
				*i2 = nullptr;
			}
			ms_staticRegions.clear();
		}

		{
			for (std::vector<const Region *>::iterator i2 = ms_dynamicRegions.begin();
				i2 != ms_dynamicRegions.end(); ++i2)
			{
				delete const_cast<Region *>(*i2);
				*i2 = nullptr;
			}
			ms_dynamicRegions.clear();
		}

		ms_installed = false;
	}
}

//-----------------------------------------------------------------------

/**
 * Reads the static region info from the planets' region datatables.
 */
void RegionMaster::readRegionDataTables()
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::readRegionDataTables, not installed"));
		return;
	}

	const char * regionFilesName = ConfigServerGame::getRegionFilesName();

	DataTable * const regionFilesTable = DataTableManager::getTable(regionFilesName, true);
	if (regionFilesTable == nullptr)
	{
		WARNING(true, ("RegionMaster::readRegionDataTables could not open file %s, static regions will not be read!", regionFilesName));
		return;
	}

	{
		const int numRegionFiles = regionFilesTable->getNumRows();
		for (int i = 0; i < numRegionFiles; ++i)
		{
			// get the region table from the filename table
			const std::string & planetName     = regionFilesTable->getStringValue(0, i);
			const std::string & regionFileName = regionFilesTable->getStringValue(1, i);
			float               regionMinX     = regionFilesTable->getFloatValue (2, i);
			float               regionMinY     = regionFilesTable->getFloatValue (3, i);
			float               regionMaxX     = regionFilesTable->getFloatValue (4, i);
			float               regionMaxY     = regionFilesTable->getFloatValue (5, i);
			
			DataTable * const regionTable = DataTableManager::getTable(regionFileName, true);
			if (regionTable == nullptr)
			{
				WARNING(true, ("RegionMaster::readRegionDataTables could not open region file %s.", regionFileName.c_str()));
				continue;
			}
			
			RegionData & regionData = ms_planetRegions[planetName];
			if (regionData.tree == nullptr)
			{
				regionData.tree = new MxCifQuadTree(regionMinX, regionMinY, regionMaxX, regionMaxY, ConfigServerGame::getRegionTreeDepth());
			}
			
			int numRegions = regionTable->getNumRows();
			for (int j = 0; j < numRegions; ++j)
			{
				// read the region data from the table
				const Unicode::String name(Unicode::narrowToWide(regionTable->getStringValue( 0, j)));
				int                 geometry      = regionTable->getIntValue   ( 1, j);
				float               minX          = regionTable->getFloatValue ( 2, j);
				float               minY          = regionTable->getFloatValue ( 3, j);
				float               maxX          = regionTable->getFloatValue ( 4, j);
				float               maxY          = regionTable->getFloatValue ( 5, j);
				int                 pvp           = regionTable->getIntValue   ( 6, j);
				int                 geography     = regionTable->getIntValue   ( 7, j);
				int                 minDifficulty = regionTable->getIntValue   ( 8, j);
				int                 maxDifficulty = regionTable->getIntValue   ( 9, j);
				int                 spawn         = regionTable->getIntValue   (10, j);
				int                 mission       = regionTable->getIntValue   (11, j);
				int                 buildable     = regionTable->getIntValue   (12, j);
				int                 municipal     = regionTable->getIntValue   (13, j);
				int                 visible       = regionTable->getIntValue   (14, j);
				int                 notify        = regionTable->getIntValue   (15, j);
				EnvironmentInfo     envFlags      = convertFromStringToEnvironmentInfo( regionTable->getStringValue(16, j) );
				
				// make sure the region has a unique name
				if (name.empty())
				{
					WARNING(true, ("RegionMaster::readRegionDataTables row %d of table %s has no region name", 
						j, regionFileName.c_str()));
					continue;
				}
				else if (regionData.nameMap.find(name) != regionData.nameMap.end())
				{
					WARNING(true, ("RegionMaster::readRegionDataTables row %d of table %s is trying to add duplicate region %s", 
						j, regionFileName.c_str(), Unicode::wideToNarrow(name).c_str()));
					continue;
				}
				
				// create a rectangular or circular region and add it to the region tree
				Region * region = nullptr;
				switch (geometry)
				{
				case RG_rectagle:
					{
						RegionRectangle * const rectangle = new RegionRectangle(minX, minY, maxX, maxY);
						if (regionData.tree->addObject(rectangle->getBounds()))
							region = rectangle;
						else
							delete rectangle;
					}
					break;
				case RG_circle:
					{
						if (pvp == PR_battlefieldPvp || pvp == PR_battlefieldPve)
						{
							RegionPvp * battlefield = new RegionPvp(minX, minY, maxX);
							if (regionData.tree->addObject(battlefield->getBounds()))
								region = battlefield;
							else
								delete battlefield;
						}
						else
						{
							RegionCircle * circle = new RegionCircle(minX, minY, maxX);
							if (regionData.tree->addObject(circle->getBounds()))
								region = circle;
							else
								delete circle;
						}
					}
					break;
				default:
					WARNING(true, ("RegionMaster::readRegionDataTables row %d of table %s has invalid geometry parameter %d", 
						j, regionFileName.c_str(), geometry));
					continue;
				}
				// set the rest of the region's data
				if (region != nullptr)
				{
					region->setName(name);
					region->setPlanet(planetName);
					region->setPvp(pvp);
					region->setGeography(geography);
					region->setMinDifficulty(minDifficulty);
					region->setMaxDifficulty(maxDifficulty);
					region->setSpawn(spawn);
					region->setMission(mission);
					region->setBuildable(buildable);
					region->setMunicipal(municipal);
					region->setVisible(visible);
					region->setNotify(notify);
					region->setEnvironmentFlags(envFlags);
					ms_staticRegions.push_back(region);
					regionData.nameMap[name] = region;
				}
				else
				{
					FATAL(true, ("RegionMaster::readRegionDataTables could not add row %d (%s) of table %s to region tree", 
						j, Unicode::wideToNarrow(name).c_str(), regionFileName.c_str()));
				}
			}
			DataTableManager::close(regionFileName);
	}
	}
	DataTableManager::close(regionFilesName);
}	// RegionMaster::readRegionDataTables

//-----------------------------------------------------------------------

/**
 * Creates a new rectangular dynamic region.
 *
 * @param minX			min x coordinate
 * @param minZ			min z coordinate
 * @param maxX			max x coordinate
 * @param maxZ			max z coordinate
 * @param name			region name
 * @param planet		region planet name
 * @param pvp			pvp state
 * @param buildable		buildable state
 * @param municipal		municipal state
 * @param geography		geography state
 * @param minDifficulty	min difficulty state
 * @param maxDifficulty	max difficulty state
 * @param spawnable		spawnable state
 * @param mission		mission state
 * @param visible		visible flag
 * @param notify        notify flag
 */
void RegionMaster::createNewDynamicRegion(float minX, float minZ, 
	float maxX, float maxZ, const Unicode::String & name, const std::string & planet, 
	int pvp, int buildable, int municipal, int geography, int minDifficulty,
	int maxDifficulty, int spawnable, int mission, bool visible, bool notify)
{
	// no creating duplicate regions
	if (getRegionByName(planet, name))
		return;

	// only the universe process may create universe objects
	if (!ServerUniverse::getInstance().isAuthoritative())
	{
		CreateDynamicRegionRectangleMessage const createMessage(minX, minZ, maxX, maxZ, name, planet, pvp, buildable, municipal, geography, minDifficulty, maxDifficulty, spawnable, mission, visible, notify);
		GameServer::getInstance().sendToCentralServer(createMessage);
		return;
	}

	// create a new UniverseObject that will be used to hold the region data
	Transform tr;
	tr.setPosition_p(Vector::xyz111);
	UniverseObject *regionObject = safe_cast<UniverseObject *>(ServerWorld::createNewObject(DYNAMIC_REGION_TEMPLATE_NAME, tr, 0, false));

	if (!regionObject)
		return;

	if (!setDynamicRegionObjectData(*regionObject, name, planet, RG_rectagle, minX, 
		minZ, maxX, maxZ, pvp, buildable, municipal, geography, minDifficulty,
		maxDifficulty, spawnable, mission, visible, notify))
	{
		regionObject->permanentlyDestroy(DeleteReasons::SetupFailed);
		return;
	}

	const Region * region = addDynamicRegion(*regionObject);
	if (region)
		regionObject->persist();
	else
	{
		regionObject->removeObjVarItem(OBJVAR_DYNAMIC_REGION);
		regionObject->permanentlyDestroy(DeleteReasons::SetupFailed);
	}
}

//----------------------------------------------------------------------

/**
 * Creates a new circlular dynamic region.
 *
 * @param centerX		x coordinate of circle center
 * @param centerZ		z coordinate of circle center
 * @param radius		circle radius
 * @param name			region name
 * @param planet		region planet name
 * @param pvp			pvp state
 * @param buildable		buildable state
 * @param municipal		municipal state
 * @param geography		geography state
 * @param minDifficulty	min difficulty state
 * @param maxDifficulty	max difficulty state
 * @param spawnable		spawnable state
 * @param mission		mission state
 * @param visible		visible flag
 * @param notify        notify flag
 *
 * @return the new region, or nullptr on error
 */
void RegionMaster::createNewDynamicRegion(float centerX, float centerZ, 
	float radius, const Unicode::String & name, const std::string & planet, int pvp, 
	int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, 
	int spawnable, int mission, bool visible, bool notify)
{
	// no creating duplicate regions
	if (getRegionByName(planet, name))
		return;

	// only the universe process may create universe objects
	if (!ServerUniverse::getInstance().isAuthoritative())
	{
		CreateDynamicRegionCircleMessage const createMessage(centerX, centerZ, radius, name, planet, pvp, buildable, municipal, geography, minDifficulty, maxDifficulty, spawnable, mission, visible, notify);
		GameServer::getInstance().sendToCentralServer(createMessage);
		return;
	}

	// create a new UniverseObject that will be used to hold the region data
	Transform tr;
	tr.setPosition_p(Vector::xyz111);
	UniverseObject *regionObject = safe_cast<UniverseObject *>(ServerWorld::createNewObject(DYNAMIC_REGION_TEMPLATE_NAME, tr, 0, false));

	if (!regionObject)
		return;

	if (!setDynamicRegionObjectData(*regionObject, name, planet, RG_circle, centerX, 
		centerZ, radius, 0, pvp, buildable, municipal, geography, minDifficulty,
		maxDifficulty, spawnable, mission, visible, notify))
	{
		regionObject->permanentlyDestroy(DeleteReasons::SetupFailed);
		return;
	}

	regionObject->addToWorld();

	const Region * region = addDynamicRegion(*regionObject);
	if (region)
		regionObject->persist();
	else
	{
		regionObject->removeObjVarItem(OBJVAR_DYNAMIC_REGION);
		regionObject->permanentlyDestroy(DeleteReasons::SetupFailed);
	}
}

//----------------------------------------------------------------------

/**
 * Sets the objvars on a universe object to turn it into a dynamic region 
 * object.
 *
 * @param object                the object
 * @param name                  region name
 * @param planet                region planet name
 * @param geometry              region geometry (rect or circle)
 * @param minX                  min x coordinate/center x
 * @param minZ                  min z coordinate/center z
 * @param maxX                  max x coordinate/radius
 * @param maxZ                  max z coordinate/ignored
 * @param pvp                   pvp state
 * @param buildable             buildable state
 * @param municipal             municipal state
 * @param geography             geography state
 * @param minDifficulty         min difficulty state
 * @param maxDifficulty         max difficulty state
 * @param spawnable             spawnable state
 * @param mission               mission state
 * @param visible				visible flag
 * @param notify                notify flag
 *
 * @return true on success, false if there was an error
 */
bool RegionMaster::setDynamicRegionObjectData(UniverseObject & object, 
	const Unicode::String & name, const std::string & planet, int geometry, 
	float minX, float minZ, float maxX, float maxZ, int pvp, int buildable, 
	int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable,
	int mission, bool visible, bool notify)
{
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_NAME, name);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_PLANET, Unicode::narrowToWide(planet));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_GEOMETRY, geometry);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MINX, minX);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MINY, minZ);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAXX, maxX);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAXY, maxZ);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_PVP, pvp);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_GEOGRAPHY, geography);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MIN_DIFFICULTY, minDifficulty);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAX_DIFFICULTY, maxDifficulty);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_SPAWN, spawnable);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MISSION, mission);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_BUILDABLE, buildable);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MUNICIPAL, municipal);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_VISIBLE, static_cast<int>(visible));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_NOTIFY, static_cast<int>(notify));
	return true;
}	// RegionMaster::setDynamicRegionObjectData

//----------------------------------------------------------------------

/**
 * Adds a dynamic region to the world.
 *
 * @param source		the source of the region
 */
const Region * RegionMaster::addDynamicRegion(const UniverseObject & source)
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::addDynamicRegion, not installed"));
		return nullptr;
	}

	DynamicVariableList::NestedList regionObjvars(source.getObjVars(),OBJVAR_DYNAMIC_REGION);

	Unicode::String planet;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_PLANET,planet))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no planet data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	Unicode::String name;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_NAME,name))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no name data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	if (name.empty())
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has empty "
			"name data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int geometry = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_GEOMETRY,geometry))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"geometry data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	float minX;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MINX,minX))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"minX data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	float minY;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MINY,minY))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"minY data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	float maxX;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MAXX,maxX))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"maxX data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	float maxY;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MAXY,maxY))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"maxY data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int pvp=0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_PVP,pvp))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"pvp data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int geography=0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_GEOGRAPHY,geography))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"geography data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int minDifficulty = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MIN_DIFFICULTY, minDifficulty))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"min difficulty data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int maxDifficulty = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MAX_DIFFICULTY, maxDifficulty))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"max difficulty data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int spawn = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_SPAWN,spawn))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"spawn data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int mission = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MISSION,mission))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"mission data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int buildable = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_BUILDABLE,buildable))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"buildable data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int municipal = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_MUNICIPAL,municipal))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"municipal data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int visible = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_VISIBLE,visible))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"visible data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	int notify = 0;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_NOTIFY,notify))
	{
		DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has no "
			"notify data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}

	// find the appropriate region data for the planet
	// note that dynamic regions cannot define new planets
	std::map<std::string, RegionData>::iterator result = ms_planetRegions.find(
		Unicode::wideToNarrow(planet));
	if (result == ms_planetRegions.end())
	{
		WARNING(true, ("RegionMaster::addDynamicRegion object %s gave unknown "
			"planet %s for its region", source.getNetworkId().getValueString().c_str(),
			Unicode::wideToNarrow(planet).c_str()));
		return nullptr;
	}
	RegionData & regionData = (*result).second;
	if (regionData.tree == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("RegionMaster::addDynamicRegion planet %s "
			"has no tree defined!", Unicode::wideToNarrow(planet).c_str()));
		return nullptr;
	}

	// make sure the region name is unique
	if (regionData.nameMap.find(name) != regionData.nameMap.end())
	{
		WARNING(true, ("RegionMaster::addDynamicRegion object %s is trying "
			"to add duplicate region %s", source.getNetworkId().getValueString().c_str(),
			Unicode::wideToNarrow(name).c_str()));
		return nullptr;
	}

	// create a rectangular or circular region and add it to the region tree
	Region * region = nullptr;
	switch (geometry)
	{
		case RG_rectagle:
			{
				RegionRectangle * rectangle = new RegionRectangle(minX, minY, maxX, maxY, CachedNetworkId(source));
				if (regionData.tree->addObject(rectangle->getBounds()))
					region = rectangle;
				else
					delete rectangle;
			}
			break;
		case RG_circle:
			{
				if (pvp == PR_battlefieldPvp || pvp == PR_battlefieldPve)
				{
					RegionPvp * battlefield = new RegionPvp(minX, minY, maxX, CachedNetworkId(source));
					if (regionData.tree->addObject(battlefield->getBounds()))
						region = battlefield;
					else
						delete battlefield;
				}
				else
				{
					RegionCircle * circle = new RegionCircle(minX, minY, maxX, CachedNetworkId(source));
					if (regionData.tree->addObject(circle->getBounds()))
						region = circle;
					else
						delete circle;
				}
			}
			break;
		default:
			DEBUG_WARNING(true, ("RegionMaster::addDynamicRegion object %s has unknown "
				"geometry type", source.getNetworkId().getValueString().c_str(),
				geometry));
			return nullptr;
	}
	if (region == nullptr)
	{
		WARNING(true, ("RegionMaster::readRegionDataTables could "
			"not add region defined by object %s to region tree", 
			source.getNetworkId().getValueString().c_str()));
		 return nullptr;
	}

	// set the rest of the region's data
	region->setName(name);
	region->setPlanet(Unicode::wideToNarrow(planet));
	region->setPvp(pvp);
	region->setGeography(geography);
	region->setMinDifficulty(minDifficulty);
	region->setMaxDifficulty(maxDifficulty);
	region->setSpawn(spawn);
	region->setMission(mission);
	region->setBuildable(buildable);
	region->setMunicipal(municipal);
	region->setVisible(visible);
	region->setNotify(notify);
	ms_dynamicRegions.push_back(region);
	regionData.nameMap[name] = region;

	if (region->asRegionPvp())
		region->asRegionPvp()->checkBattlefieldMarker();

	// if the region is a notify region, force checkNotifyRegions() to be
	// called on all character creatures in the region as soon as possible so
	// notifications will be sent that the creature has entered the region
	if (notify)
	{
		std::vector<const Region *> regions;

		CreatureObject::AllCreaturesSet const &creatureList = CreatureObject::getAllCreatures();
		for (CreatureObject::AllCreaturesSet::const_iterator i = creatureList.begin(); i != creatureList.end(); ++i)
		{
			CreatureObject *creatureObj = const_cast<CreatureObject*>(*i);
			if (creatureObj->isInitialized() && creatureObj->isAuthoritative() && creatureObj->isPlayerControlled() && creatureObj->isInWorld())
			{
				regions.clear();
				getRegionsAtPoint(ServerWorld::getSceneId(), creatureObj->getPosition_w().x, creatureObj->getPosition_w().z, regions);
				if (std::find(regions.begin(), regions.end(), region) != regions.end())
					creatureObj->forceNotifyRegionsCheck();
			}
		}
	}

	return region;
}	// RegionMaster::addDynamicRegion

//----------------------------------------------------------------------

/**
 * Removes a dynamic region from the world.
 *
 * @param source		the source of the region
 */
void RegionMaster::removeDynamicRegion(const UniverseObject & source)
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::removeDynamicRegion, not installed"));
		return;
	}

	DynamicVariableList::NestedList regionObjvars(source.getObjVars(),OBJVAR_DYNAMIC_REGION);

	Unicode::String planet;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_PLANET,planet))
	{
		DEBUG_WARNING(true, ("RegionMaster::removeDynamicRegion object %s has no "
			"planet data", source.getNetworkId().getValueString().c_str()));
		return;
	}
	Unicode::String name;
	if (!regionObjvars.getItem(OBJVAR_DYNAMIC_REGION_NAME,name))
	{
		DEBUG_WARNING(true, ("RegionMaster::removeDynamicRegion object %s has no "
			"name data", source.getNetworkId().getValueString().c_str()));
		return;
	}

	// find the appropriate region data for the planet
	// note that dynamic regions cannot define new planets
	std::map<std::string, RegionData>::iterator dataResult = ms_planetRegions.find(
		Unicode::wideToNarrow(planet));
	if (dataResult == ms_planetRegions.end())
	{
		WARNING(true, ("RegionMaster::removeDynamicRegion object %s gave unknown "
			"planet %s for its region", source.getNetworkId().getValueString().c_str(),
			Unicode::wideToNarrow(planet).c_str()));
		return;
	}
	RegionData & regionData = (*dataResult).second;
	if (regionData.tree == nullptr)
	{
		WARNING_STRICT_FATAL(true, ("RegionMaster::addDynamicRegion planet %s "
			"has no tree defined!", Unicode::wideToNarrow(planet).c_str()));
		return;
	}

	// find the region by its name
	RegionsMappedByName::iterator regionResult = regionData.nameMap.find(name);
	if (regionResult == regionData.nameMap.end())
	{
		WARNING(true, ("RegionMaster::removeDynamicRegion object %s gave unknown "
			"name %s for its region", source.getNetworkId().getValueString().c_str(),
			Unicode::wideToNarrow(name).c_str()));
		return;
	}
	const Region * region = (*regionResult).second;
	if (region && !region->isDynamic())
	{
		WARNING(true, ("RegionMaster::removeDynamicRegion object %s tried to remove static region %s",
			source.getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(name).c_str()));
		return;
	}

	// if the region is a notify region, force checkNotifyRegions() to be
	// called on all character creatures in the region as soon as possible so
	// notifications will be sent that the creature has left the region
	if (region->getNotify())
	{
		std::vector<const Region *> regions;

		CreatureObject::AllCreaturesSet const &creatureList = CreatureObject::getAllCreatures();
		for (CreatureObject::AllCreaturesSet::const_iterator i = creatureList.begin(); i != creatureList.end(); ++i)
		{
			CreatureObject *creatureObj = const_cast<CreatureObject*>(*i);
			if (creatureObj->isInitialized() && creatureObj->isAuthoritative() && creatureObj->isPlayerControlled() && creatureObj->isInWorld())
			{
				regions.clear();
				getRegionsAtPoint(ServerWorld::getSceneId(), creatureObj->getPosition_w().x, creatureObj->getPosition_w().z, regions);
				if (std::find(regions.begin(), regions.end(), region) != regions.end())
					creatureObj->forceNotifyRegionsCheck();
			}
		}
	}

	regionData.nameMap.erase(regionResult);

	// remove the region from the tree
	if (!regionData.tree->removeObject(region->getBounds()))
	{
		WARNING(true, ("RegionMaster::removeDynamicRegion error removing dynamic "
			"region %s from region tree, continuing anyway",
			source.getNetworkId().getValueString().c_str()));
	}
	
	// remove the region from the dynamic region list
	std::vector<const Region *>::iterator removeResult = std::find(
		ms_dynamicRegions.begin(), ms_dynamicRegions.end(), region);
	if (removeResult != ms_dynamicRegions.end())
	{
		ms_dynamicRegions.erase(removeResult);
	}
	else
	{
		WARNING(true, ("RegionMaster::removeDynamicRegion dynamic region %s was "
			"not in the dynamic region list, continuing anyway",
			source.getNetworkId().getValueString().c_str()));
	}

	// remove the region
	delete region;
}	// RegionMaster::removeDynamicRegion

//----------------------------------------------------------------------

/**
 * Returns the dynamic region associated with a UniverseObject.
 *
 * @param source		the object to get the region from
 */
const Region * RegionMaster::getDynamicRegionFromObject(const UniverseObject & source)
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::getDynamicRegionFromObject, not installed"));
		return nullptr;
	}

	DynamicVariableList::NestedList regionObjVars(source.getObjVars(),OBJVAR_DYNAMIC_REGION);

	Unicode::String planet;
	if (!regionObjVars.getItem(OBJVAR_DYNAMIC_REGION_PLANET,planet))
	{
		DEBUG_WARNING(true, ("RegionMaster::getDynamicRegionFromObject object %s has no "
			"planet data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}
	Unicode::String name;
	if (!regionObjVars.getItem(OBJVAR_DYNAMIC_REGION_NAME,name))
	{
		DEBUG_WARNING(true, ("RegionMaster::getDynamicRegionFromObject object %s has no "
			"name data", source.getNetworkId().getValueString().c_str()));
		return nullptr;
	}

	return getRegionByName(Unicode::wideToNarrow(planet), name);
}	// RegionMaster::getDynamicRegionFromObject

//----------------------------------------------------------------------

/**
 * Returns the region with the given name.
 */
Region * RegionMaster::getRegionByName(const std::string & planetName, const Unicode::String & regionName)
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::getRegionByName, not installed"));
		return nullptr;
	}

	const RegionData & regionData = ms_planetRegions[planetName];
	if (regionData.tree != nullptr)
	{
		RegionsMappedByName::const_iterator result = regionData.nameMap.find(regionName);
		if (result != regionData.nameMap.end())
			return (*result).second;
	}
	return nullptr;
}	// RegionMaster::getRegionByName

//----------------------------------------------------------------------

/**
 * Finds the region with the smallest area at a given point.
 */
const Region * RegionMaster::getSmallestRegionAtPoint(const std::string & planet, float x, float z)
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::getSmallestRegionAtPoint, not installed"));
		return nullptr;
	}

	std::vector<const Region *> regions;
	getRegionsAtPoint(planet, x, z, regions);
	
	float area = 0;
	const Region * region = nullptr;
	for (std::vector<const Region *>::const_iterator iter = regions.begin();
		iter != regions.end(); ++iter)
	{
		if (region == nullptr)
		{
			region = *iter;
			area = region->getArea();
		}
		else
		{
			float testArea = (*iter)->getArea();
			if (testArea < area)
			{
				area = testArea;
				region = *iter;
			}
		}
	}
	return region;
}	// RegionMaster::getSmallestRegionAtPoint

//----------------------------------------------------------------------

/**
 * Same as getSmallestRegionAtPoint, but the region must be marked as visible.
 */
const Region * RegionMaster::getSmallestVisibleRegionAtPoint(const std::string & planet, float x, float z)
{
	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::getSmallestRegionAtPoint, not installed"));
		return nullptr;
	}

	std::vector<const Region *> regions;
	getRegionsAtPoint(planet, x, z, regions);
	
	float area = 0;
	const Region * region = nullptr;
	for (std::vector<const Region *>::const_iterator iter = regions.begin(); iter != regions.end(); ++iter)
	{
		if ((*iter)->isVisible())
		{
			if (region == nullptr)
			{
				region = *iter;
				area = region->getArea();
			}
			else
			{
				float testArea = (*iter)->getArea();
				if (testArea < area)
				{
					area = testArea;
					region = *iter;
				}
			}
		}
	}
	return region;
}	// RegionMaster::getSmallestVisibleRegionAtPoint

//----------------------------------------------------------------------

/**
 * Finds the regions at a given point.
 *
 * @param planet		planet name of region to get
 * @param x				x coordinate of point
 * @param z				z coordinate of point
 * @param regions		vector to be filled in with list of regions at the point
 */
void RegionMaster::getRegionsAtPoint(const std::string & planet, float x, float z, 
	std::vector<const Region *> & regions)
{
	regions.clear();

	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::getRegionsAtPoint, not installed"));
		return;
	}

	const RegionData & regionData = ms_planetRegions[planet];
	if (regionData.tree != nullptr)
	{
		std::vector<const MxCifQuadTreeBounds *> objects;
		regionData.tree->getObjectsAt(x, z, objects);
		for (std::vector<const MxCifQuadTreeBounds *>::const_iterator iter = objects.begin();
			iter != objects.end(); ++iter)
		{
			regions.push_back(static_cast<Region *>((*iter)->getData()));
		}
	}
}	// RegionMaster::getRegionsAtPoint

//----------------------------------------------------------------------

/**
 * Returns all the regions for a given planet.
 */
void RegionMaster::getRegionsForPlanet(const std::string & planetName, 
	std::vector<const Region *> & regions)
{
	regions.clear();

	if (!ms_installed)
	{
		WARNING(true, ("RegionMaster::getRegionsForPlanet, not installed"));
		return;
	}

	const RegionData & regionData = ms_planetRegions[planetName];
	if (regionData.tree != nullptr)
	{
		std::vector<const MxCifQuadTreeBounds *> objects;
		regionData.tree->getAllObjects(objects);
		for (std::vector<const MxCifQuadTreeBounds *>::const_iterator iter = objects.begin();
			iter != objects.end(); ++iter)
		{
			regions.push_back(static_cast<Region *>((*iter)->getData()));
		}
	}
}	// RegionMaster::getRegionsForPlanet

//----------------------------------------------------------------------

void RegionMaster::registerBattlefieldMarker(const std::string &regionName, BattlefieldMarkerObject *marker)
{
	Region *region = getRegionByName(ServerWorld::getSceneId(), Unicode::narrowToWide(regionName));
	if (region)
	{
		RegionPvp *regionPvp = region->asRegionPvp();
		if (regionPvp)
			regionPvp->setBattlefieldMarker(marker);
	}
}

EnvironmentInfo RegionMasterNamspace::convertFromStringToEnvironmentInfo(std::string const & stringList)
{
	if(stringList.empty())
		return 0;

	if(s_conversionMap.empty())
	{
		// Fill the Datatable with values from our environment flags datatable.
		DataTable * const environmentFlagTable = DataTableManager::getTable(s_environmentFlagsDataTable, true);
		for(int i = 0; i < environmentFlagTable->getNumRows(); ++i)
		{
			s_conversionMap.insert( std::pair<std::string, EnvironmentInfo>(environmentFlagTable->getStringValue(0,i), static_cast<EnvironmentInfo>(1 << i) ) );
		}
	}

	std::string parserList = stringList;
	const char delimiter = ',';
	EnvironmentInfo outputValue = 0;
	
	// Parse the input string using , as the delimiter.
	std::vector<std::string> parsedStringVector;
	std::string::size_type loc = parserList.find(delimiter);

	if (std::string::npos != loc) // We found a comma
	{
		std::vector<int> locations;
		locations.push_back(loc);
		while (std::string::npos != (loc = parserList.find(delimiter, loc+1)))
		{
			locations.push_back(loc);
		}

		for(std::vector<int>::size_type i = 0; i < locations.size(); ++i)
		{
			std::string subString;

			if(i == 0)
				subString = parserList.substr(0, locations[i]);
			else
				subString = parserList.substr(locations[i - 1], locations[i]);

			parsedStringVector.push_back(subString);
		}

	}
	else
		parsedStringVector.push_back(parserList); // No comma, must be one item in the list.

	// Now we just search our map for the strings we just read in. If we find it, we add that flag to our output value.
	for(std::vector<std::string>::size_type i = 0; i < parsedStringVector.size(); ++i)
	{
		std::map<std::string, EnvironmentInfo>::iterator searchIter = s_conversionMap.find(parsedStringVector[i]);

		if( searchIter != s_conversionMap.end())
		{
			outputValue |= searchIter->second;
		}
	}


	return outputValue;
}

NetworkId RegionMaster::createNewDynamicRegionWithSpawn(float centerX, float centerY, float centerZ,
										  float radius, const Unicode::String & name, const std::string & planet, int pvp, 
										  int buildable, int municipal, int geography, int minDifficulty, int maxDifficulty, 
										  int spawnable, int mission, bool visible, bool notify, std::string spawnDatatable, int duration)
{
	// no creating duplicate regions
	if (getRegionByName(planet, name))
		return NetworkId();

	// only the universe process may create universe objects
	if (!ServerUniverse::getInstance().isAuthoritative())
	{
		CreateDynamicSpawnRegionCircleMessage const createMessage(centerX, centerY, centerZ, radius, name, planet, pvp, buildable, municipal, geography, minDifficulty, maxDifficulty, spawnable, mission, visible, notify, spawnDatatable, duration);
		GameServer::getInstance().sendToCentralServer(createMessage);
		return NetworkId();
	}

	// create a new UniverseObject that will be used to hold the region data
	Transform tr;
	tr.setPosition_p(Vector::xyz111);
	UniverseObject *regionObject = safe_cast<UniverseObject *>(ServerWorld::createNewObject(DYNAMIC_REGION_TEMPLATE_NAME, tr, 0, false));

	if (!regionObject || !regionObject->getScriptObject())
		return NetworkId();

	if (!setDynamicSpawnRegionObjectData(*regionObject, name, planet, RG_circle, centerX, 
		centerZ, radius, 0, pvp, buildable, municipal, geography, minDifficulty,
		maxDifficulty, spawnable, mission, visible, notify, spawnDatatable, duration))
	{
		regionObject->permanentlyDestroy(DeleteReasons::SetupFailed);
		return NetworkId();
	}

	regionObject->addToWorld();

	const Region * region = addDynamicRegion(*regionObject);
	if (region)
		regionObject->persist();
	else
	{
		regionObject->removeObjVarItem(OBJVAR_DYNAMIC_REGION);
		regionObject->permanentlyDestroy(DeleteReasons::SetupFailed);
	}

	// Everything went okay. Add dynamic spawn script and other objvars.
	regionObject->getScriptObject()->attachScript(s_dynamicMasterScript, true);

	if(!spawnDatatable.empty())
	{
		// Creature spawning has to be done in script.
		ScriptParams params;
		params.addParam(regionObject->getNetworkId());
		params.addParam(spawnDatatable.c_str());
		params.addParam(centerX);
		params.addParam(centerY);
		params.addParam(centerZ);

		IGNORE_RETURN(regionObject->getScriptObject()->trigAllScripts(Scripting::TRIG_ON_DYNAMIC_SPAWN_REGION_CREATED, params));
	}

	return regionObject->getNetworkId();
}

bool RegionMaster::setDynamicSpawnRegionObjectData(UniverseObject & object, 
											  const Unicode::String & name, const std::string & planet, int geometry, 
											  float minX, float minZ, float maxX, float maxZ, int pvp, int buildable, 
											  int municipal, int geography, int minDifficulty, int maxDifficulty, int spawnable,
											  int mission, bool visible, bool notify, std::string spawntable, int duration)
{
	time_t const birthEpoch = ::time(nullptr);
	time_t const endEpoch = birthEpoch + (duration * 60); // Duration is in minutes.

	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_NAME, name);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_PLANET, Unicode::narrowToWide(planet));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_GEOMETRY, geometry);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MINX, minX);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MINY, minZ);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAXX, maxX);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAXY, maxZ);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_PVP, pvp);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_GEOGRAPHY, geography);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MIN_DIFFICULTY, minDifficulty);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAX_DIFFICULTY, maxDifficulty);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_SPAWN, spawnable);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MISSION, mission);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_BUILDABLE, buildable);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MUNICIPAL, municipal);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_VISIBLE, static_cast<int>(visible));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_NOTIFY, static_cast<int>(notify));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_SPAWNTABLE, spawntable);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_BIRTH, static_cast<int>(birthEpoch));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_DURATION, duration);
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_MAXDURATION, duration + (duration/3)); // Max duration is an extra 30% of the time.
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_READABLE_BIRTH, CalendarTime::convertEpochToTimeStringGMT_YYYYMMDDHHMMSS(birthEpoch));
	object.setObjVarItem(OBJVAR_DYNAMIC_REGION + "." + OBJVAR_DYNAMIC_REGION_READABLE_DEATH, CalendarTime::convertEpochToTimeStringGMT_YYYYMMDDHHMMSS(endEpoch));
	return true;
}	// RegionMaster::setDynamicRegionObjectData

//----------------------------------------------------------------------

