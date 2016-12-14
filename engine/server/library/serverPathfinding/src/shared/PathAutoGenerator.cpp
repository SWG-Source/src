//======================================================================
//
// PathAutoGenerator.cpp
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/PathAutoGenerator.h"

#include "UnicodeUtils.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedMath/MxCifQuadTreeBounds.h"
#include "sharedObject/CellProperty.h"
#include "sharedTerrain/TerrainObject.h"
#include <vector>

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

#define USE_OBSTACLE_TEMPLATE 0

//----------------------------------------------------------------------

namespace PathAutoGeneratorNamespace
{
	char const * const s_pathWaypointTemplate = "object/path_waypoint/path_waypoint.iff";
}

using namespace PathAutoGeneratorNamespace;

//----------------------------------------------------------------------

Region const * PathAutoGenerator::findPathRegion(Vector const & pos_w)
{
	std::string const & planetName = ServerWorld::getSceneId();
	
	RegionMaster::RegionVector rv;
	RegionMaster::getRegionsAtPoint(planetName, pos_w.x, pos_w.z, rv);
	
	for (RegionMaster::RegionVector::const_iterator it = rv.begin(); it != rv.end(); ++it)
	{
		Region const * const r = *it;
		if (nullptr != r)
		{
			if (r->getGeography() == RegionNamespace::RG_pathfind)
				return r;
		}
	}
	
	return nullptr;
}

//----------------------------------------------------------------------

void PathAutoGenerator::pathAutoGenerate(Vector const & pos_w, float nodeDistance, float obstacleDistance, Unicode::String & result)
{
	Region const * region = findPathRegion(pos_w);
	if (nullptr == region)
	{
		result += Unicode::narrowToWide("No pathfinding region at position");
		return;
	}
	
	char buf[128];
	size_t const bufsize = sizeof(buf);
	snprintf(buf, bufsize, "Auto generating path for region [%s]\n", Unicode::wideToNarrow(region->getName()).c_str());
	result += Unicode::narrowToWide(buf);
	
	MxCifQuadTreeBounds const & bounds = region->getBounds();
	
	float const minX = bounds.getMinX();
	float const minY = bounds.getMinY();
	float const maxX = bounds.getMaxX();
	float const maxY = bounds.getMaxY();
	
	TerrainObject const * const terrain = TerrainObject::getConstInstance();
	
	Vector testPos_w;
	
	Vector const goalOffsets[] =
	{
		//-- north
		Vector(0.0f, 0.0f, obstacleDistance),
			//-- east
			Vector(obstacleDistance, 0.0f, 0.0f),
			//-- south
			Vector(0.0f, 0.0f, -obstacleDistance),
			//-- west
			Vector(-obstacleDistance, 0.0f, 0.0f)
	};
	
	int const numGoalOffsets = sizeof(goalOffsets) / sizeof(*goalOffsets);
	
	{
		for (testPos_w.x = minX; testPos_w.x < (maxX); testPos_w.x += nodeDistance)
		{
			for (testPos_w.z = minY; testPos_w.z < (maxY); testPos_w.z += nodeDistance)
			{
				terrain->isPassableForceChunkCreation(testPos_w);
			}
		}
	}
	
	int const maxCreateCount = 500;
	
	int createCount = 0;
	int obstacleNearbySkipped = 0;
	
	for (testPos_w.x = minX; testPos_w.x < maxX; testPos_w.x += nodeDistance)
	{
		for (testPos_w.z = minY; testPos_w.z < maxY; testPos_w.z += nodeDistance)
		{
			if (!bounds.isPointIn(testPos_w.x, testPos_w.z))
				continue;
			
			if (!terrain->isPassable(testPos_w))
				continue;
			
			bool skip = false;
			
			for (int i = 0; i < numGoalOffsets; ++i)
			{
				Vector const & goalPos_w = testPos_w + goalOffsets[i];
				CanMoveResult const cmr = 
					CollisionWorld::canMove(CellProperty::getWorldCellProperty(), testPos_w, goalPos_w, 1.0f, false, false, false);
				
				if (cmr != CMR_MoveOK)
				{
					skip = true;
					break;
				}
			}
			
			if (skip)
			{
				++obstacleNearbySkipped;

#if USE_OBSTACLE_TEMPLATE
				if (nullptr != PathAutoGeneratorNamespace::s_pathObstacleTemplate)
				{
					Transform transform_w;
					transform_w.setPosition_p(testPos_w);
					ServerObject * newObject = ServerWorld::createNewObject(s_pathObstacleTemplate, transform_w, 0, false);
					newObject->addToWorld();
				}
#endif
				continue;
			}
			
			if (createCount >= maxCreateCount)
			{
				++createCount;
			}
			
			Transform transform_w;
			transform_w.setPosition_p(testPos_w);
			ServerObject * newObject = ServerWorld::createNewObject(s_pathWaypointTemplate, transform_w, 0, false);
			
			if (nullptr != newObject)
			{
				newObject->addToWorld();
				newObject->persist();
				++createCount;
			}
			
		}
	}
	
	snprintf(buf, bufsize, "Nodes created: %d, skipped %d\n", createCount, obstacleNearbySkipped);
	result += Unicode::narrowToWide(buf);
	
	if (createCount > maxCreateCount)
	{
		snprintf(buf, bufsize, "  Max nodes (%d) hit, try changing your parameters\n", maxCreateCount);
		result += Unicode::narrowToWide(buf);
	}
}
	
//----------------------------------------------------------------------

void PathAutoGenerator::pathAutoCleanup(Vector const & pos_w, Unicode::String & result)
{
	Region const * region = findPathRegion(pos_w);
	if (nullptr == region)
	{
		result += Unicode::narrowToWide("No pathfinding region at position");
		return;
	}
	
	
	char buf[128];
	size_t const bufsize = sizeof(buf);
	snprintf(buf, bufsize, "Cleanup up pathnodes for region [%s]\n", Unicode::wideToNarrow(region->getName()).c_str());
	result += Unicode::narrowToWide(buf);
	
	MxCifQuadTreeBounds const & bounds = region->getBounds();
	
	float const minX = bounds.getMinX();
	float const minY = bounds.getMinY();
	float const maxX = bounds.getMaxX();
	float const maxY = bounds.getMaxY();
	
	float const range = std::max(maxX - minX, maxY - minY);
	
	Vector const center((minX + maxX) * 0.5f, 0.0f, (minY + maxY) * 0.5f);
	
	typedef std::vector<ServerObject *> ServerObjectVector;
	
	ServerObjectVector sv;
	
	ServerWorld::findObjectsInRange(center, range,  sv);
	
	int destroyCount = 0;
	int obstacleDestroyCount = 0;
	for (ServerObjectVector::const_iterator it = sv.begin(); it != sv.end(); ++it)
	{
		ServerObject * const so = *it;
		
		Vector const & pos_w = so->getPosition_w();
		
		if (bounds.isPointIn(pos_w.x, pos_w.z))
		{
			if (!strcmp(so->getTemplateName(), s_pathWaypointTemplate))
			{
				so->permanentlyDestroy(DeleteReasons::Script);
				++destroyCount;
				continue;
			}

#if USE_OBSTACLE_TEMPLATE
			if (nullptr != s_pathObstacleTemplate && !strcmp(so->getTemplateName(), s_pathObstacleTemplate))
			{
				so->permanentlyDestroy(DeleteReasons::Script);
				++obstacleDestroyCount;
				continue;
			}
#endif
		}
	}
	
	snprintf(buf, bufsize, "Nodes destroyed: %d, obstacle placeholders destroyed: %d\n", destroyCount, obstacleDestroyCount);
	result += Unicode::narrowToWide(buf);
}

//----------------------------------------------------------------------
