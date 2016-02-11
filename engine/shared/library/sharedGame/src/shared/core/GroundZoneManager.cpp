//======================================================================
//
// GroundZoneManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h" 
#include "sharedGame/GroundZoneManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <map>

//=====================================================================

namespace GroundZoneManagerNamespace
{

	static bool                                       s_installed          = false;
	static std::string const                          ms_zoneSizesFile("datatables/instance/instance_areas.iff");

	typedef std::map<std::string, std::pair<float, float> > ZoneSizeMap;

	static ZoneSizeMap                   ms_zoneSizes;
	static std::map<std::string, bool>   ms_adjustForNonSquare;

	static std::string                   cms_sceneAndAreaDelimeter("__");
}

using namespace GroundZoneManagerNamespace;

//----------------------------------------------------------------------

void GroundZoneManager::install ()
{
	InstallTimer const installTimer("GroundZoneManager::install");

	DEBUG_FATAL (s_installed, ("already installed.\n"));

	ms_zoneSizes.clear();
	ms_adjustForNonSquare.clear();
	
	DataTable * table = DataTableManager::getTable(ms_zoneSizesFile, true);
	if(table)
	{
		int const numRows = table->getNumRows ();

		for (int i = 0; i < numRows; ++i)
		{
			const std::string &areaName           = table->getStringValue("area", i);
			const float        x                  = table->getFloatValue("x", i);
			const float        z                  = table->getFloatValue("z", i);
			const bool         adjustForNonSquare = table->getIntValue("adjustForNonSquare", i) != 0;

			FATAL(z == 0, ("z CANNOT be 0.0f"));

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

			ZoneSizeMap::iterator result = ms_zoneSizes.insert(
				ZoneSizeMap::value_type(
					areaName, 
					std::pair<float, float>(x, z)
				)
			).first;

			const std::string &actualKeyString = result->first;
			ms_adjustForNonSquare[actualKeyString] = adjustForNonSquare;

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		}

		DataTableManager::close(ms_zoneSizesFile);
	}
	
	ExitChain::add(remove, "GroundZoneManager::remove");
	s_installed = true;
}

//----------------------------------------------------------------------

void GroundZoneManager::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	ms_zoneSizes.clear();
	ms_adjustForNonSquare.clear();

	s_installed = false;
}

//----------------------------------------------------------------------

BuildoutArea const * GroundZoneManager::getZoneName(std::string const & sceneName, Vector const & location_w, std::string & zoneName)
{
	zoneName = sceneName;

	if(!SharedBuildoutAreaManager::isBuildoutScene(sceneName))
	{
		return nullptr;
	}	
	else
	{
		BuildoutArea const * const ba = SharedBuildoutAreaManager::findBuildoutAreaAtPosition(sceneName, location_w.x, location_w.z, true);
		if (nullptr != ba)
		{
			if (!ba->compositeName.empty())
				zoneName = ba->compositeName;
			else
				zoneName = ba->areaName;
		}
		return ba;
	}
}

//----------------------------------------------------------------------

Vector GroundZoneManager::transformWorldLocationToZoneLocation(std::string const & sceneName, Vector const & location_w)
{
	Vector pos_w = location_w;
	std::string zoneName;
	BuildoutArea const * const ba = GroundZoneManager::getZoneName(sceneName, location_w, zoneName);
	if (nullptr != ba)
	{
		pos_w = ba->getRelativePosition(location_w, true);
	}

	//if if this scene needs to be adjusted
	if(!SharedBuildoutAreaManager::adjustMapCoordinatesForScene(sceneName))
	{
		return pos_w;
	}

	if (ba)
		zoneName = sceneName + cms_sceneAndAreaDelimeter + zoneName;

	return adjustForNonSquareZones(zoneName, pos_w);
}

//----------------------------------------------------------------------

Vector GroundZoneManager::adjustForNonSquareZones(std::string const & encodedZoneName, Vector const & zoneLocation)
{
	Vector result = zoneLocation;	
	
	static std::string currentZone;
	static Vector2d zoneSize;
	static float xToZRatio = 1.0f;

	if (encodedZoneName != currentZone)
	{
		currentZone = encodedZoneName;
		zoneSize    = getZoneSize(currentZone.c_str());
		xToZRatio   = getXToZRatioOfZone(currentZone.c_str());
	}
	
	if (ms_adjustForNonSquare[currentZone.c_str()])
	{
		// now adjust for non-square zones

		//scale
		if(xToZRatio != 1.0f)
		{
			if(xToZRatio > 1.0f)
				result.z /= xToZRatio;
			else
				result.x *= xToZRatio;

			//offset
			if(zoneSize.x > zoneSize.y)
			{
				result.z -= ((zoneSize.x - zoneSize.y)/2);
			}
			else if(zoneSize.x < zoneSize.y)
			{
				result.x -= ((zoneSize.y - zoneSize.x)/2);
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

Vector2d GroundZoneManager::getZoneSize(std::string const & encodedZoneName)
{
	const ZoneSizeMap::const_iterator zi = ms_zoneSizes.find(encodedZoneName);
	if (zi != ms_zoneSizes.end())
	{
		std::pair<float, float> const & size = zi->second;
		return Vector2d(size.first, size.second);
	}
	
	return SharedBuildoutAreaManager::getBuildoutAreaSize(encodedZoneName, true);
}

//----------------------------------------------------------------------

float GroundZoneManager::getXToZRatioOfZone(std::string const &  zoneName)
{
	const ZoneSizeMap::const_iterator zi = ms_zoneSizes.find(zoneName);
	if (zi != ms_zoneSizes.end())
	{
		std::pair<float, float> const & size = zi->second;
		return size.first / size.second;
	}
	else
	{
		//any zone not in the map is assumed to be square (i.e. regular planets)
		return 1.0f;
	}
}

//----------------------------------------------------------------------

float GroundZoneManager::getXToZRatioOfZoneFromScene(std::string const & sceneName, Vector const & location_w)
{
	std::string zoneName;
	getZoneName(sceneName, location_w, zoneName);
	return getXToZRatioOfZone(zoneName.c_str());
}

//----------------------------------------------------------------------

Vector GroundZoneManager::getRelativePositionFromPlayer(const char *sceneName, Vector const & playerPosition_w, Vector const & worldPosition_w)
{
	Vector relativePosition(worldPosition_w);
	
	if (sceneName && *sceneName)
	{
		if (SharedBuildoutAreaManager::adjustMapCoordinatesForScene(sceneName))
		{
			std::string worldZoneName;
			if (GroundZoneManager::getZoneName(sceneName, worldPosition_w, worldZoneName))
			{
				std::string encodedZoneName = SharedBuildoutAreaManager::createEncodedBuildoutName(sceneName, worldZoneName);

				relativePosition = SharedBuildoutAreaManager::getRelativePositionInArea(encodedZoneName.c_str(), worldPosition_w, TRUE);
				
				std::string playerZoneName;
				if (GroundZoneManager::getZoneName(sceneName, playerPosition_w, playerZoneName))
				{
					encodedZoneName = SharedBuildoutAreaManager::createEncodedBuildoutName(sceneName, playerZoneName);
					relativePosition += SharedBuildoutAreaManager::getCenterPositionInArea(encodedZoneName.c_str(), playerPosition_w);
				}
			}
		}
	}
	return relativePosition;
}

//----------------------------------------------------------------------

Vector GroundZoneManager::getRelativePosition(const char *sceneName, Vector const & worldPosition_w)
{
	Vector relativePosition(worldPosition_w);

	if (sceneName && *sceneName)
	{
		if (SharedBuildoutAreaManager::adjustMapCoordinatesForScene(sceneName))
		{
			std::string zoneName;
			if (GroundZoneManager::getZoneName(sceneName, worldPosition_w, zoneName))
			{
				relativePosition = SharedBuildoutAreaManager::getRelativePositionInArea(zoneName.c_str(), worldPosition_w, TRUE);
			}
		}
	}

	return relativePosition;
}

//======================================================================

