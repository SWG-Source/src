// ======================================================================
//
// SharedBuildoutAreaManager.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SharedBuildoutAreaManager.h"

#include "Archive/Archive.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMathArchive/QuaternionArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"

// ======================================================================

namespace SharedBuildoutAreaManagerNamespace
{
	// ----------------------------------------------------------------------

	static bool s_installed = false;
	
	// ----------------------------------------------------------------------
	typedef std::map<std::string, std::vector<BuildoutArea> > BuildoutAreaNameMap;
	typedef std::map<std::string, bool>                       BoolNameMap;
	
	static std::vector<std::string>                                       s_buildoutScenes;
	static BoolNameMap                                                    s_adjustMapCoordinates;
	static BuildoutAreaNameMap                                            s_buildoutAreas;
	
	static std::string                                                    s_currentScene;
	static const std::vector<BuildoutArea>                               *s_buildoutAreasForCurrentScene;
	static const std::vector<BuildoutArea>                                s_emptyBuildoutAreas;

	static std::string                                                    cms_sceneAndAreaDelimeter("__");

	// ----------------------------------------------------------------------

	static float getPlanetEdgeSize()
	{
		TerrainObject const * const terrain = TerrainObject::getInstance ();
		if (terrain)
		{
			ProceduralTerrainAppearance const * const pta = dynamic_cast<ProceduralTerrainAppearance const *>(terrain->getAppearance ());
			if (pta)
				return pta->getMapWidthInMeters();
		}
		//return default planet size
		return 16384.0f;
	}

	// ----------------------------------------------------------------------
}
using namespace SharedBuildoutAreaManagerNamespace;

// ======================================================================

int BuildoutArea::getSharedBaseId() const
{
	return -(areaIndex+1)*30000;
}

// ----------------------------------------------------------------------

int BuildoutArea::getSharedTopId() const
{
	return getSharedBaseId()+29999;
}

// ----------------------------------------------------------------------

int BuildoutArea::getServerBaseId() const
{
	return -(areaIndex+1)*30000-1000000;
}

// ----------------------------------------------------------------------

int BuildoutArea::getServerTopId() const
{
	return getServerBaseId()+29999;
}

// ----------------------------------------------------------------------

bool BuildoutArea::isLocationInside(float x, float z) const
{
	return rect.isWithin(x, z);
}

// ----------------------------------------------------------------------

float BuildoutArea::getXCenterPoint(bool allowComposite) const
{
	if (allowComposite && getUseComposite())
		return compositeRect.x0 + ((compositeRect.x1 - compositeRect.x0) * 0.5f);

	return rect.x0 + ((rect.x1 - rect.x0) * 0.5f);
}

// ----------------------------------------------------------------------

float BuildoutArea::getZCenterPoint(bool allowComposite) const
{
	if (allowComposite && getUseComposite())
		return compositeRect.y0 + ((compositeRect.y1 - compositeRect.y0) * 0.5f);

	return rect.y0 + ((rect.y1 - rect.y0) * 0.5f);
}

//----------------------------------------------------------------------

Rectangle2d const & BuildoutArea::getRectangle(bool allowComposite) const
{
	if (allowComposite && getUseComposite())
		return compositeRect;
	return rect;
}

//----------------------------------------------------------------------

Vector2d BuildoutArea::getSize(bool allowComposite) const
{
	Rectangle2d const & r = getRectangle(allowComposite);
	return Vector2d(r.getWidth(), r.getHeight());
}

//----------------------------------------------------------------------

bool BuildoutArea::getUseComposite() const
{
	return !compositeName.empty();
}

//----------------------------------------------------------------------

Vector BuildoutArea::getRelativePosition(Vector const & pos_w, bool allowComposite) const
{
	Vector resultPos;

	resultPos.x = pos_w.x - getXCenterPoint(allowComposite);
	resultPos.y = pos_w.y;
	resultPos.z = pos_w.z - getZCenterPoint(allowComposite);

	return resultPos;
}

//----------------------------------------------------------------------

std::string const & BuildoutArea::getRequiredEventName() const
{
	return requiredEventName;
}

//----------------------------------------------------------------------

std::string const & BuildoutArea::getRequiredLoadLevel() const
{
	return requiredLoadLevel;
}

// ======================================================================

bool SharedBuildoutAreaManager::isBuildoutScene(std::string const & sceneName)
{
	for (int i = 0; i < static_cast<int>(s_buildoutScenes.size()); ++i)
	{
		if (s_buildoutScenes[i] == sceneName)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void SharedBuildoutAreaManager::install()
{
	s_installed = true;

	s_buildoutScenes.clear();
	s_adjustMapCoordinates.clear();

	s_buildoutAreasForCurrentScene=&s_emptyBuildoutAreas;
	s_buildoutAreas.clear();

	Iff iff;
	if (iff.open("datatables/buildout/buildout_scenes.iff", true))
	{
		DataTable buildoutScenesTable;
		buildoutScenesTable.load(iff);

		int const sceneCount = buildoutScenesTable.getNumRows();
		for (int sceneRow = 0; sceneRow < sceneCount; ++sceneRow)
		{
			const std::string &sceneNameTemp = buildoutScenesTable.getStringValue("sceneName", sceneRow);
			s_buildoutScenes.push_back(sceneNameTemp);
			const std::string &sceneName = s_buildoutScenes.back();

			const int adjustMapCoordinates = buildoutScenesTable.getIntValue("adjust_map_coordinates", sceneRow);

			s_adjustMapCoordinates.insert(std::make_pair(sceneName, adjustMapCoordinates!=0));
		}
	}

  	const int numBuildoutScenes = static_cast<int>(s_buildoutScenes.size());
  	for (int i = 0; i < numBuildoutScenes; ++i)
	{
		char filename[256];
		std::string const & buildoutScene = s_buildoutScenes[i];
  
  		IGNORE_RETURN(snprintf(filename, sizeof(filename)-1, "datatables/buildout/areas_%s.iff", buildoutScene.c_str()));

		Iff iff;
		if (iff.open(filename, true))
		{
			DataTable areaListTable;
			areaListTable.load(iff);

			std::vector<BuildoutArea> areasForScene;

			int const areaCount = areaListTable.getNumRows();
			for (int areaRow = 0; areaRow < areaCount; ++areaRow)
			{
				areasForScene.emplace_back(BuildoutArea());
				BuildoutArea &buildoutArea = areasForScene.back();
				buildoutArea.areaIndex = i*100+areaRow;
				buildoutArea.areaName = areaListTable.getStringValue("area", areaRow);
				buildoutArea.rect.x0 = areaListTable.getFloatValue("x1", areaRow);
				buildoutArea.rect.y0 = areaListTable.getFloatValue("z1", areaRow);
				buildoutArea.rect.x1 = areaListTable.getFloatValue("x2", areaRow);
				buildoutArea.rect.y1 = areaListTable.getFloatValue("z2", areaRow);
				buildoutArea.useClipRect = areaListTable.getIntValue("useClipRect", areaRow) != 0;
				buildoutArea.clipRect.x0 = areaListTable.getFloatValue("clipRectX1", areaRow);
				buildoutArea.clipRect.y0 = areaListTable.getFloatValue("clipRectZ1", areaRow);
				buildoutArea.clipRect.x1 = areaListTable.getFloatValue("clipRectX2", areaRow);
				buildoutArea.clipRect.y1 = areaListTable.getFloatValue("clipRectZ2", areaRow);

				if (buildoutArea.useClipRect)
				{
					//-- if a zero clip rect was supplied, just use the buildout rect
					if (buildoutArea.clipRect.getWidth() <= 0.0f || buildoutArea.clipRect.getHeight() <= 0.0f)
						buildoutArea.clipRect = buildoutArea.rect;
				}

				buildoutArea.clipEnvironmentFlags = areaListTable.getIntValue("envFlags", areaRow);
				buildoutArea.clipEnvironmentFlags &= ~areaListTable.getIntValue("envFlagsExclude", areaRow);

				buildoutArea.compositeName = areaListTable.getStringValue("compositeName", areaRow);
				if (!buildoutArea.compositeName.empty())
				{
					buildoutArea.compositeRect.x0 = areaListTable.getFloatValue("compositeX1", areaRow);
					buildoutArea.compositeRect.y0 = areaListTable.getFloatValue("compositeZ1", areaRow);
					buildoutArea.compositeRect.x1 = areaListTable.getFloatValue("compositeX2", areaRow);
					buildoutArea.compositeRect.y1 = areaListTable.getFloatValue("compositeZ2", areaRow);
				}

				buildoutArea.isolated = areaListTable.getIntValue("isolated", areaRow) != 0;
				buildoutArea.allowMap = areaListTable.getIntValue("allowMap", areaRow) != 0;
				buildoutArea.allowRadarTerrain = areaListTable.getIntValue("allowRadarTerrain", areaRow) != 0;

				buildoutArea.internalBuildoutArea = areaListTable.getIntValue("internal", areaRow) != 0;

				buildoutArea.requiredEventName = areaListTable.getStringValue("eventRequired", areaRow);

				buildoutArea.requiredLoadLevel = areaListTable.getStringValue("requiredLoadLevel", areaRow);
			}

  			s_buildoutAreas[buildoutScene] = areasForScene;
		}
		else
		{
			WARNING(true, ("SharedBuildoutAreaManager unable to load areas file [%s]", filename));
		}
	}

	ExitChain::add(remove, "SharedBuildoutAreaManager::remove");
}


//----------------------------------------------------------------------

void SharedBuildoutAreaManager::remove()
{
	s_buildoutScenes.clear();
	s_adjustMapCoordinates.clear();
	s_buildoutAreasForCurrentScene=&s_emptyBuildoutAreas;
	s_buildoutAreas.clear();
}

//----------------------------------------------------------------------

void SharedBuildoutAreaManager::load(std::string const & sceneName)
{
	if (sceneName == s_currentScene)
		return;

	s_currentScene = sceneName;

	BuildoutAreaNameMap::const_iterator const it = s_buildoutAreas.find(sceneName);
	
	if (it != s_buildoutAreas.end())
	{
		s_buildoutAreasForCurrentScene = &(it->second);
	}
	else
	{
		s_buildoutAreasForCurrentScene=&s_emptyBuildoutAreas;
	}	
}

// ----------------------------------------------------------------------

std::vector<BuildoutArea> const &SharedBuildoutAreaManager::getBuildoutAreasForCurrentScene()
{
	return *s_buildoutAreasForCurrentScene;
}

// ----------------------------------------------------------------------

std::string SharedBuildoutAreaManager::getBuildoutNameForPosition(std::string const & sceneName, Vector const & location_w, bool ignoreInternal, bool ignoreNonActiveEvents)
{
	BuildoutArea const * const ba = findBuildoutAreaAtPosition(sceneName, location_w.x, location_w.z, ignoreInternal, ignoreNonActiveEvents);
	
	if (nullptr != ba)
		return sceneName + cms_sceneAndAreaDelimeter + ba->areaName;

	return sceneName;
}

// ----------------------------------------------------------------------

Vector SharedBuildoutAreaManager::getRelativePositionInArea(std::string const & buildoutName, Vector const & location_w, bool ignoreInternal, bool ignoreNonActiveEvents)
{
	Vector resultPos = location_w;
	std::pair<std::string, std::string> const & sceneAndArea = parseEncodedBuildoutName(buildoutName);
	if (!sceneAndArea.second.empty())
	{
		if (s_buildoutAreas.find(sceneAndArea.first) != s_buildoutAreas.end())
		{
			std::vector<BuildoutArea> const & areas = s_buildoutAreas[sceneAndArea.first];
			for(std::vector<BuildoutArea>::const_iterator i = areas.begin(); i != areas.end(); ++i)
			{
				BuildoutArea const & boa = *i;
				
				if (ignoreInternal && boa.internalBuildoutArea)
					continue;

				if(ignoreNonActiveEvents && !boa.requiredEventName.empty()) // Eventually this needs to query the scheduler system and see if the event is currently active.
					continue;

				if(ignoreNonActiveEvents && !boa.requiredLoadLevel.empty())
					continue;
				
				if(boa.isLocationInside(location_w.x, location_w.z) && boa.areaName == sceneAndArea.second)
				{
					return boa.getRelativePosition(location_w, true);
				}
			}
		}
	}
	
	
	return resultPos;
}

// ----------------------------------------------------------------------

Vector SharedBuildoutAreaManager::getCenterPositionInArea(const char *buildoutName, Vector const & location_w)
{
	Vector resultPos(Vector::zero);
	if (buildoutName && *buildoutName)
	{
		std::pair<std::string, std::string> sceneAndArea = parseEncodedBuildoutName(buildoutName);
		if (!sceneAndArea.second.empty())
		{
			if (s_buildoutAreas.find(sceneAndArea.first.c_str()) != s_buildoutAreas.end())
			{
				std::vector<BuildoutArea> const & areas = s_buildoutAreas[sceneAndArea.first.c_str()];
				for(std::vector<BuildoutArea>::const_iterator i = areas.begin(); i != areas.end(); ++i)
				{
					if (i->isLocationInside(location_w.x, location_w.z) && i->areaName == sceneAndArea.second)
					{
						resultPos.x = i->getXCenterPoint(false);
						resultPos.y = 0.0f;
						resultPos.z = i->getZCenterPoint(false);
						break;
					}
				}
			}
		}
	}

	return resultPos;
}

// ----------------------------------------------------------------------

//Vector SharedBuildoutAreaManager::getRelativePositionFromWorldSpaceToPlayerSpace(std::string const & zoneName, Vector const & playerPosition_w, Vector const & worldPosition_w)
//{
//	Vector localPosition(getRelativePositionInArea(zoneName, worldPosition_w));
//	localPosition += getCenterPositionInArea(zoneName, playerPosition_w);

//	return localPosition;
//}

// ----------------------------------------------------------------------

SharedBuildoutAreaManager::StringPair SharedBuildoutAreaManager::parseEncodedBuildoutName(std::string const & encodedBuildoutName)
{
	const size_t underscore_pos = encodedBuildoutName.find(cms_sceneAndAreaDelimeter);
	if(underscore_pos == encodedBuildoutName.npos)
	{
		//we're on a regular planet, return (planet_name, "")
		return std::make_pair(encodedBuildoutName, std::string());
	}

	std::string const & sceneName = encodedBuildoutName.substr (0, underscore_pos);
	std::string const & areaName  = encodedBuildoutName.substr (underscore_pos + cms_sceneAndAreaDelimeter.size());
	return std::make_pair(sceneName, areaName);
}

// ----------------------------------------------------------------------

std::string SharedBuildoutAreaManager::createEncodedBuildoutName(std::string const & sceneName, std::string const & zoneName)
{
	if (!zoneName.empty())
		return sceneName + cms_sceneAndAreaDelimeter + zoneName;

	return sceneName;
}

// ----------------------------------------------------------------------

Vector2d SharedBuildoutAreaManager::getBuildoutAreaSize(std::string const & encodedBuildoutName, bool allowComposite)
{
	float const planetEdgeSize = getPlanetEdgeSize();
	std::pair<std::string, std::string> sceneAndArea = parseEncodedBuildoutName(encodedBuildoutName);
	if(sceneAndArea.second.empty())
	{
		return Vector2d(planetEdgeSize, planetEdgeSize);
	}
	else
	{
		if (s_buildoutAreas.find(sceneAndArea.first) != s_buildoutAreas.end())
		{
			std::vector<BuildoutArea> const & areas = s_buildoutAreas[sceneAndArea.first];
			for(std::vector<BuildoutArea>::const_iterator i = areas.begin(); i != areas.end(); ++i)
			{
				BuildoutArea const & b = *i;
				
				//return the size of the named area
				if(b.areaName == sceneAndArea.second)
				{
					return b.getSize(allowComposite);
				}
			}
		}
	}
	
	//failure case, return whole planet size
	return Vector2d(planetEdgeSize, planetEdgeSize);
}

// ----------------------------------------------------------------------

bool SharedBuildoutAreaManager::adjustMapCoordinatesForScene(std::string const & sceneName)
{
	BoolNameMap::const_iterator const i = s_adjustMapCoordinates.find(sceneName);
	if(i != s_adjustMapCoordinates.end())
		return i->second;
	else
		return false;
}

// ======================================================================

void Archive::get(Archive::ReadIterator &source, ServerBuildoutAreaRow &target)
{
	Archive::get(source, target.id);
	Archive::get(source, target.container);
	Archive::get(source, target.serverTemplateCrc);
	Archive::get(source, target.cellIndex);
	Archive::get(source, target.position);
	Archive::get(source, target.orientation);
	Archive::get(source, target.scripts);
	Archive::get(source, target.objvars);
}

// ----------------------------------------------------------------------

void Archive::put(Archive::ByteStream &target, ServerBuildoutAreaRow const &source)
{
	Archive::put(target, source.id);
	Archive::put(target, source.container);
	Archive::put(target, source.serverTemplateCrc);
	Archive::put(target, source.cellIndex);
	Archive::put(target, source.position);
	Archive::put(target, source.orientation);
	Archive::put(target, source.scripts);
	Archive::put(target, source.objvars);
}

// ----------------------------------------------------------------------

void Archive::get(Archive::ReadIterator &source, ClientBuildoutAreaRow &target)
{
	Archive::get(source, target.id);
	Archive::get(source, target.container);
	Archive::get(source, target.type);
	Archive::get(source, target.sharedTemplateCrc);
	Archive::get(source, target.cellIndex);
	Archive::get(source, target.position);
	Archive::get(source, target.orientation);
	Archive::get(source, target.radius);
	Archive::get(source, target.portalLayoutCrc);
}

// ----------------------------------------------------------------------

void Archive::put(Archive::ByteStream &target, ClientBuildoutAreaRow const &source)
{
	Archive::put(target, source.id);
	Archive::put(target, source.container);
	Archive::put(target, source.type);
	Archive::put(target, source.sharedTemplateCrc);
	Archive::put(target, source.cellIndex);
	Archive::put(target, source.position);
	Archive::put(target, source.orientation);
	Archive::put(target, source.radius);
	Archive::put(target, source.portalLayoutCrc);
}

//----------------------------------------------------------------------

SharedBuildoutAreaManager::BuildoutAreaVector const * SharedBuildoutAreaManager::findBuildoutAreasForScene(std::string const & sceneName)
{
	BuildoutAreaNameMap::const_iterator const it = s_buildoutAreas.find(sceneName);
	
	if (it != s_buildoutAreas.end())
	{
		return &it->second;
	}
	return nullptr;
}

//----------------------------------------------------------------------

BuildoutArea const * SharedBuildoutAreaManager::findBuildoutAreaAtPosition(std::string const & sceneId, float x, float z, bool ignoreInternal, bool ignoreNonActiveEvents)
{
	BuildoutAreaVector const * const bav = findBuildoutAreasForScene(sceneId);

	if (nullptr == bav)
		return nullptr;

	for (BuildoutAreaVector::const_iterator it = bav->begin(); it != bav->end(); ++it)
	{
		BuildoutArea const & buildoutArea = *it;

		if (ignoreInternal && buildoutArea.internalBuildoutArea)
		{
			continue;
		}

		if(ignoreNonActiveEvents && !buildoutArea.requiredEventName.empty())
			continue;

		if(ignoreNonActiveEvents && !buildoutArea.requiredLoadLevel.empty())
			continue;

		if (buildoutArea.isLocationInside(x, z))
		{
			return &buildoutArea;
		}
	}

	return nullptr;
}
//----------------------------------------------------------------------

BuildoutArea const * SharedBuildoutAreaManager::findBuildoutAreaAtPosition(float x, float z, bool ignoreInternal,bool ignoreNonActiveEvents)
{
	for (std::vector<BuildoutArea>::const_iterator it = s_buildoutAreasForCurrentScene->begin(); it != s_buildoutAreasForCurrentScene->end(); ++it)
	{
		BuildoutArea const & buildoutArea = *it;

		if (ignoreInternal && buildoutArea.internalBuildoutArea)
		{
			return nullptr;
		}

		if(ignoreNonActiveEvents && !buildoutArea.requiredEventName.empty())
			return nullptr;

		if(ignoreNonActiveEvents && !buildoutArea.requiredLoadLevel.empty())
			return nullptr;

		if (buildoutArea.isLocationInside(x, z))
		{
			return &buildoutArea;
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------

std::string const & BuildoutArea::getReferenceDisplayName() const
{
	if (getUseComposite())
	{
		return compositeName;
	}

	return areaName;
}

//----------------------------------------------------------------------

bool SharedBuildoutAreaManager::isTraversable(std::string const & sceneName, Vector const & location_w1, Vector const & location_w2)
{
	if (!isBuildoutScene(sceneName))
		return true;

	BuildoutArea const * const ba1 = findBuildoutAreaAtPosition(sceneName, location_w1.x, location_w1.z, true);
	BuildoutArea const * const ba2 = findBuildoutAreaAtPosition(sceneName, location_w2.x, location_w2.z, true);

	//-- check if the second location is isolated from the first non-buildout location
	if (!ba1)
	{
		if (ba2 && ba2->isolated)
			return false;

		return true;
	}

	//-- check if the first location is isolated from the second non-buildout location
	if (!ba2)
	{
		if (ba1 && ba1->isolated)
			return false;

		return true;
	}

	//-- actual same areas (pointer comparison)
	if (ba1 == ba2)
		return true;

	//-- same area names
	if (ba1->areaName == ba2->areaName)
		return true;

	//-- is either location isolated
	if (!ba1->isolated && !ba2->isolated)
		return true;

	//-- otherwise, non-traversable
	return false;
}

// ======================================================================
