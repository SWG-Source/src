//======================================================================
//
// PlanetMapManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/PlanetMapManager.h"

#include "UnicodeUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/DataTable.h"
#include <map>
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedFoundation/Crc.h"

//======================================================================

namespace PlanetMapManagerNamespace
{
	typedef std::map<uint8,       std::string>     TypeNameMap;
	typedef std::map<std::string, uint8>           NameTypeMap;
	typedef std::map<uint8,       StringId> TypeStringIdMap;

	typedef std::pair<uint32, bool> FactionAndVisibility;
	typedef std::map<uint8,       FactionAndVisibility>          TypeFactionMap;

	TypeNameMap    s_typeNameMap;
	NameTypeMap    s_nameTypeMap;
	TypeStringIdMap  s_typeStringMap;
	TypeFactionMap s_typeFactionMap;

	bool s_installed = false;

	uint8 s_cityCategory     = 0;
	uint8 s_waypointCategory = 0;
	uint8 s_poiCategory      = 0;
	uint8 s_gcwRegionCategory = 0;

	enum DtCols
	{
		DC_name,
		DC_index,
		DC_isCategory,
		DC_isSubCategory,
		DC_canBeActive,
		DC_faction,
		DC_factionVisibleOnly
	};

	void install ()
	{
		if (s_installed)
			return;
		
		s_installed = true;
		
		{
			static const char * const filename = "datatables/player/planet_map_cat.iff";
			Iff iff;
			
			if (!iff.open (filename, true))
				WARNING (true, ("Data file %s not available.", filename));
			else
			{
				DataTable dt;
				dt.load (iff);
				iff.close ();
				
				const int numRows = dt.getNumRows ();
				
				static const std::string table = "map_loc_cat_n";

				for (int i = 0; i < numRows; ++i)
				{
					const std::string &     name    = dt.getStringValue (DC_name,   i);
					const uint8             index   = static_cast<uint8>(dt.getIntValue    (DC_index,  i));
					const StringId & str            = StringId (table, name);
					
					const std::string &     faction = dt.getStringValue (DC_faction, i);

					s_typeNameMap.insert   (std::make_pair (index, name));
					s_nameTypeMap.insert   (std::make_pair (name,  index));
					s_typeStringMap.insert (std::make_pair (index, str));
					
					if (name == "city")
						s_cityCategory = index;
					else if (name == "waypoint")
						s_waypointCategory = index;
					else if (name == "poi")
						s_poiCategory = index;
					else if (name == "gcw_region")
						s_gcwRegionCategory = index;

					if (!faction.empty ())
					{
						const bool factionVisibleOnly = dt.getIntValue (DC_factionVisibleOnly, i) != 0;
						s_typeFactionMap.insert (std::make_pair (index, FactionAndVisibility (Crc::normalizeAndCalculate (faction.c_str ()), factionVisibleOnly)));
					}
				}
			}

			DEBUG_FATAL (!s_cityCategory,     ("PlanetMapManager has no city category"));
			DEBUG_FATAL (!s_waypointCategory, ("PlanetMapManager has no waypoint category"));
		}
	}
}

using namespace PlanetMapManagerNamespace;

//----------------------------------------------------------------------

bool PlanetMapManager::checkCategoryFaction (uint8 category, uint32 & faction, bool & factionVisibilityOnly)
{
	if (!s_installed)
		install ();

	const TypeFactionMap::const_iterator it = s_typeFactionMap.find (category);
	if (it != s_typeFactionMap.end ())
	{
		const FactionAndVisibility & fav = (*it).second;
		faction = fav.first;
		factionVisibilityOnly = fav.second;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

Vector PlanetMapManager::convertPositionToFictional   (const std::string & sceneId, const Vector & pos_w)
{
	if (!s_installed)
		install ();

	UNREF (sceneId);
	return pos_w;
}

//----------------------------------------------------------------------

Vector PlanetMapManager::convertPositionFromFictional (const std::string & sceneId, const Vector & fictionalPos_w)
{
	if (!s_installed)
		install ();

	UNREF (sceneId);
	return fictionalPos_w;
}

//----------------------------------------------------------------------

const std::string & PlanetMapManager::findCategoryName     (uint8 category)
{
	if (!s_installed)
		install ();

	const TypeNameMap::const_iterator it = s_typeNameMap.find (category);
	if (it != s_typeNameMap.end ())
		return (*it).second;

	static const std::string emptyString;
	return emptyString;
}

//----------------------------------------------------------------------

const StringId & PlanetMapManager::findCategoryStringId   (uint8 category)
{
	if (!s_installed)
		install ();

	const TypeStringIdMap::const_iterator it = s_typeStringMap.find (category);
	if (it != s_typeStringMap.end ())
		return (*it).second;

	return StringId::cms_invalid;
}

//----------------------------------------------------------------------

uint8 PlanetMapManager::findCategory         (const std::string & name, bool warn)
{
	if (!s_installed)
		install ();

	if (name.empty ())
		return 0;

	const NameTypeMap::const_iterator it = s_nameTypeMap.find (name);
	if (it != s_nameTypeMap.end ())
		return (*it).second;

	UNREF(warn);
	DEBUG_WARNING (warn, ("MapLocation category [%s] invalid", name.c_str ()));

	return 0;
}

//----------------------------------------------------------------------

MapLocation PlanetMapManager::makeMapLocation (const NetworkId &locationId, const Unicode::String &locationName, const Vector2d &location, const std::string &categoryName, const std::string &subCategoryName, int flags, bool & result)
{
	const uint8 category    = PlanetMapManager::findCategory (categoryName);
	const uint8 subCategory = PlanetMapManager::findCategory (subCategoryName);

	result = category && (subCategory || subCategoryName.empty ());

	WARNING (!result, ("PlanetMapManager::makeMapLocation failed for id=[%s], name=[%s], cat=[%s], subcat=[%s], flags=[%d]", 
		locationId.getValueString ().c_str (), Unicode::wideToNarrow (locationName).c_str (), categoryName.c_str (), subCategoryName.c_str (), flags));

	return MapLocation (locationId, locationName, location, category, subCategory, static_cast<uint8>(flags));
}

//----------------------------------------------------------------------

uint8 PlanetMapManager::getCityCategory      ()
{
	if (!s_installed)
		install ();

	return s_cityCategory;
}

//----------------------------------------------------------------------

uint8 PlanetMapManager::getWaypointCategory  ()
{
	if (!s_installed)
		install ();

	return s_waypointCategory;
}

//----------------------------------------------------------------------

uint8 PlanetMapManager::getPoiCategory  ()
{
	if (!s_installed)
		install ();

	return s_poiCategory;
}

//----------------------------------------------------------------------

uint8 PlanetMapManager::getGCWRegionCategory  ()
{
	if (!s_installed)
		install ();

	return s_gcwRegionCategory;
}

//----------------------------------------------------------------------

bool PlanetMapManager::sceneHasSupermap(std::string const & sceneId)
{
	return (sceneId.find("kashyyyk") != sceneId.npos);
}

//----------------------------------------------------------------------

std::string PlanetMapManager::getSceneSupermap(std::string const & sceneId)
{
	if(sceneId.find("kashyyyk") != sceneId.npos)
		return "kashyyyk_planet";
	else
		return "";
}

//======================================================================
