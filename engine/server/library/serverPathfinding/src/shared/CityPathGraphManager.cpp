// ======================================================================
//
// CityPathGraphManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/CityPathGraphManager.h"

#include "serverGame/BuildingObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"

#include "serverPathfinding/CityPathGraph.h"
#include "serverPathfinding/CityPathNode.h"
#include "serverPathfinding/ServerPathfindingConstants.h"
#include "serverPathfinding/ServerPathfindingMessaging.h"
#include "serverPathfinding/ServerPathfindingNotification.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/Containment3d.h"

#include "sharedDebug/PerformanceTimer.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/NetworkId.h"

#include "sharedMath/Vector2d.h"
#include "sharedMath/Vector.h"
#include "sharedMath/MultiShape.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/MxCifQuadTreeBounds.h"
#include "sharedMath/ShapeUtils.h"

#include "sharedObject/CellProperty.h"

#include "sharedPathfinding/ConfigSharedPathfinding.h"

#include "sharedTerrain/TerrainObject.h"

#include "StringId.h"

#include <vector>
#include <algorithm>
#include <map>
#include <set>

struct RegionCacheEntry
{
	RegionCacheEntry ( void )
	: m_shape(), m_graph(nullptr)
	{
	}

	RegionCacheEntry ( MultiShape const & shape, CityPathGraph * graph )
	: m_shape(shape), m_graph(graph)
	{
	}

	MultiShape      m_shape;
	CityPathGraph * m_graph;
};

typedef std::vector<RegionCacheEntry> RegionCache;
typedef std::vector<NetworkId> NetworkIdList;
typedef std::vector<CityPathNode*> NodeList;

namespace CityPathGraphManagerNamespace
{
	StringId g_emptyStringId;

	Unicode::String g_emptyString;
	Unicode::String g_defaultString(Unicode::narrowToWide("default"));

	float g_linkDistance;

	RegionCache g_regionCache;

	int g_scrubberGraphIndex = 0;
	int g_scrubberNodeIndex = 0;
	int g_scrubberPassCount = 0;
};

using namespace CityPathGraphManagerNamespace;

// ======================================================================

void CityPathGraphManager::install ( void )
{
}

void CityPathGraphManager::remove ( void )
{
	int count = g_regionCache.size();

	for(int i = 0; i < count; i++)
	{
		delete g_regionCache[i].m_graph;
	}

	g_regionCache.clear();
}

// ----------------------------------------------------------------------

extern float pathScrubTime;

void CityPathGraphManager::update ( float time )
{
	pathScrubTime = 0.0f;

	PerformanceTimer timer;

	timer.start();

	UNREF(time);

	if(ConfigSharedPathfinding::getEnablePathScrubber())
	{
		// Scrub the path graphs - check for bad nodes and fix them

		if(g_regionCache.size() == 0) return;

		int graphCount = g_regionCache.size();

		if(g_scrubberGraphIndex >= graphCount)
		{
			DEBUG_REPORT_LOG(true,("CityPathGraphManager path scrubber pass %d complete\n",g_scrubberPassCount));

			g_scrubberGraphIndex = 0;
			return;
		}

		CityPathGraph * graph = g_regionCache[g_scrubberGraphIndex].m_graph;

		if(graph == nullptr) return;

		if(g_scrubberNodeIndex >= graph->getNodeCount())
		{
			g_scrubberNodeIndex = 0;
			g_scrubberGraphIndex++;
			return;
		}

		CityPathNode * node = graph->_getNode(g_scrubberNodeIndex);

		g_scrubberNodeIndex++;

		if(node == nullptr) return;

		if(!node->sanityCheck(false))
		{
			graph->relinkNode(node->getIndex());
			node->saveAllData();
			ServerPathfindingMessaging::getInstance().sendNodeInfo(node);
		}
	}

	timer.stop();

	pathScrubTime = timer.getElapsedTime();
}

// ----------------------------------------------------------------------

void CityPathGraphManager::sanityCheck ( void )
{
	int count = g_regionCache.size();

	for(int i = 0; i < count; i++)
	{
		g_regionCache[i].m_graph->sanityCheck(true);
	}
}

// ----------------------------------------------------------------------

class RegionTest
{
public:

	int m_type;

	RegionTest( int type ) : m_type(type) {}

	bool operator () ( Region const * r ) const
	{
		return r->getGeography() != m_type;
	}
};

// ----------------------------------------------------------------------

bool getRegionsWithTypeAtPoint( Vector const & point, int type, std::vector<const Region *> & results )
{
	RegionMaster::getRegionsAtPoint(ServerWorld::getSceneId(), point.x, point.z, results);
	
	if(!results.empty())
	{
		results.erase(std::remove_if(results.begin(),results.end(),RegionTest(type)),results.end());
	}

	return !results.empty();
}

// ----------------------------------------------------------------------

Region const * getCityRegionFor( Vector const & position )
{
	static std::vector<const Region *> results;

	getRegionsWithTypeAtPoint( position, CITY_REGION_TYPE, results );

	if(!results.empty())
	{
		return results[0];
	}

	getRegionsWithTypeAtPoint( position, PATHFINDING_REGION_TYPE, results );

	if(!results.empty())
	{
		return results[0];
	}

	return nullptr;
}

// ----------------------------------------------------------------------

int getCityTokenFor ( Region const * region )
{
	if(region == nullptr) return -1;

	Unicode::String const & name = region->getName();

	if(name.empty() || (name == g_defaultString))
	{
		return -1;
	}
	else
	{
		int token = static_cast<int>(Crc::calculate(Unicode::wideToNarrow(name).c_str()));

		if(token == -1)
		{
			DEBUG_FATAL(true,("CityPathGraphManager::getCityTokenFor - City name hashed to -1 (shouldn't happen)\n"));
		}

		return token;
	}
}

// ----------

int getCityTokenFor( Vector const & position )
{
	Region const * region = getCityRegionFor(position);

	return getCityTokenFor(region);
}

// ----------------------------------------------------------------------

bool checkRegionCache ( Vector const & position, CityPathGraph * & outGraph )
{
	int count = g_regionCache.size();

	for(int i = 0; i < count; i++)
	{
		RegionCacheEntry & entry = g_regionCache[i];

		if(Containment3d::Test(position,entry.m_shape))
		{
			outGraph = entry.m_graph;
			return true;
		}
	}

	return false;
}

// ----------

bool addToRegionCache ( MultiShape const & shape, CityPathGraph * graph )
{
	if(graph == nullptr) return false;

	RegionCacheEntry entry(shape,graph);

	g_regionCache.push_back(entry);

	return true;
}

// ----------

bool addToRegionCache ( Cylinder const & cylinder, CityPathGraph * graph )
{
	MultiShape multiShape(cylinder);

	return addToRegionCache(multiShape,graph);
}

// ----------

bool addToRegionCache ( AxialBox const & box, CityPathGraph * graph )
{
	MultiShape multiShape(box);

	return addToRegionCache(multiShape,graph);
}

// ----------

bool addToRegionCache ( Region const * region, CityPathGraph * graph )
{
	if(region == nullptr) return false;
	if(graph == nullptr) return false;

	MxCifQuadTreeBounds const * bounds = &region->getBounds();

	if(bounds == nullptr) return false;

	MxCifQuadTreeCircleBounds const * circleBounds = dynamic_cast<MxCifQuadTreeCircleBounds const *>(bounds);

	if(circleBounds)
	{
		Vector base( circleBounds->getCenterX(), -2000.0f, circleBounds->getCenterY() );
		float radius = circleBounds->getRadius();

		Cylinder cylinder(base,radius,4000.0f);

		return addToRegionCache(cylinder,graph);
	}
	else
	{
		Vector min( bounds->getMinX(), -2000.0f, bounds->getMinY() );
		Vector max( bounds->getMaxX(), 2000.0f, bounds->getMaxY() );

		AxialBox box(min,max);

		return addToRegionCache(box,graph);
	}
}

// ----------

bool removeFromRegionCache ( CityPathGraph * graph )
{
	int count = g_regionCache.size();

	for(int i = 0; i < count; i++)
	{
		RegionCacheEntry & entry = g_regionCache[i];

		if(entry.m_graph == graph)
		{
			entry = g_regionCache.back();
			g_regionCache.resize(count - 1);
			return true;
		}
	}

	return false;
}

// ----------

CityPathGraph * createCityGraphFor ( MultiShape const & shape )
{
	CityPathGraph * newGraph = new CityPathGraph(-1);

	addToRegionCache(shape,newGraph);

	return newGraph;
}

CityPathGraph * createCityGraphFor ( Region const * region )
{
	int token = getCityTokenFor(region);

	if(token == -1) return nullptr;

	CityPathGraph * graph = new CityPathGraph(token);

	addToRegionCache(region,graph);

	return graph;
}

CityPathGraph * createCityGraphFor ( ServerObject const * creator, MultiShape const & shape )
{
	if(creator == nullptr) return nullptr;

	CityPathGraph * newGraph = new CityPathGraph(-1);

	newGraph->setCreator(creator->getNetworkId());

	addToRegionCache(shape,newGraph);

	return newGraph;
}

// ----------------------------------------------------------------------

CityPathGraph * _getCityGraphFor ( Vector const & position )
{
	CityPathGraph * graph = nullptr;

	if(checkRegionCache(position,graph))
	{
		return graph;
	}
	else
	{
		Region const * region = getCityRegionFor(position);

		graph = createCityGraphFor(region);

		return graph;
	}
}

// ----------

CityPathGraph * _getCityGraphFor ( ServerObject const * object )
{
	if(object == nullptr)
	{
		return nullptr;
	}
	else
	{
		return _getCityGraphFor(object->getPosition_w());
	}
}

// ----------------------------------------------------------------------

CityPathNode * _getCityNodeFor ( ServerObject const * object, CityPathGraph * graph )
{
	if(object == nullptr) return nullptr;
	if(graph == nullptr) return nullptr;

	return graph->findNodeForObject(*object);
}

// ----------

CityPathNode * _getCityNodeFor ( ServerObject const * object )
{
	CityPathGraph * graph = _getCityGraphFor(object);

	if(graph == nullptr) return nullptr;

	return _getCityNodeFor(object,graph);
}

// ----------------------------------------------------------------------

PathGraph const * _getExteriorGraph ( ServerObject const * object )
{
	if(object == nullptr) return nullptr;

	CollisionProperty const * collision = object->getCollisionProperty();

	if(collision == nullptr) return nullptr;

	Floor const * floor = collision->getFloor();

	if(floor == nullptr) return nullptr;

	FloorMesh const * floorMesh = floor->getFloorMesh();

	if(floorMesh == nullptr) return nullptr;

	PathGraph const * pathGraph = safe_cast<PathGraph const *>(floorMesh->getPathGraph());

	return pathGraph;
}

// ----------------------------------------------------------------------

CityPathGraph const * CityPathGraphManager::getCityGraphFor ( ServerObject const * object )
{
	return _getCityGraphFor(object);
}

CityPathGraph const * CityPathGraphManager::getCityGraphFor ( Vector const & position )
{
	return _getCityGraphFor(position);
}

// ----------------------------------------------------------------------

CityPathNode const * CityPathGraphManager::getNamedNodeFor( ServerObject const & object, Unicode::String const & nodeName )
{
	CityPathGraph const * cityGraph = getCityGraphFor(&object);
	if (cityGraph == nullptr)
		return nullptr;

	return cityGraph->findNearestNodeForName(nodeName, object.getPosition_w());
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::getClosestPathNodePos ( ServerObject const * object, Vector & outPos )
{
	if(object == nullptr) return false;

	if(object->getParentCell() == CellProperty::getWorldCellProperty()) 
	{
		CityPathGraph const * graph = getCityGraphFor(object);

		if(graph)
		{
			int closestIndex = graph->findNearestNode(object->getPosition_p());

			if(closestIndex != -1)
			{
				PathNode const * node = graph->getNode(closestIndex);

				if(node) 
				{
					outPos = node->getPosition_p();

					return true;
				}
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

void CityPathGraphManager::addWaypoint ( ServerObject * sourceObject )
{
	if(sourceObject == nullptr) return;

	CityPathGraph * graph = _getCityGraphFor(sourceObject);

	if(graph == nullptr) 
	{
		return;
	}

	// ----------

	DynamicVariableList const & objvars = sourceObject->getObjVars();

	CityPathNode * newNode = nullptr;

	Vector sourcePos_w = sourceObject->getPosition_w();

	float height = 0.0f;

	if(TerrainObject::getConstInstance ()->getHeightForceChunkCreation(sourcePos_w,height))
	{
		if(height > sourcePos_w.y)
		{
			sourcePos_w.y = height;
		}
	}

	NetworkId sourceId = sourceObject->getNetworkId();

	NetworkId creatorId;

	if( objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_CREATOR,creatorId) )
	{
		newNode = new CityPathNode(sourcePos_w,sourceId,creatorId);
	}
	else
	{
		newNode = new CityPathNode(sourcePos_w,sourceId);
	}


	if(objvars.hasItem(OBJVAR_PATHFINDING))
	{
		// minor hack - make objects that don't have their node type set default to CityWaypoint
		
		int type = PNT_Invalid;

		objvars.getItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE,type);

		if(type == PNT_Invalid)
		{
			sourceObject->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE, static_cast<int>(PNT_CityWaypoint) );
		}

		newNode->loadInfoFromObjvars();
	}
	else
	{
		newNode->setType(PNT_CityWaypoint);
		newNode->setKey(-1);

		newNode->saveInfoToObjvars();
	}

	// ----------
	
	IGNORE_RETURN( graph->addNode(newNode) );

	ServerPathfindingMessaging::getInstance().sendNodeInfo(newNode);
	ServerPathfindingMessaging::getInstance().sendNeighborInfo(newNode);
}

// ----------------------------------------------------------------------

void CityPathGraphManager::removeWaypoint ( ServerObject * sourceObject )
{
	CityPathGraph * graph = _getCityGraphFor(sourceObject);

	if(graph == nullptr) return;

	CityPathNode * unloadingNode = _getCityNodeFor(sourceObject,graph);

	if(unloadingNode == nullptr) return;

	// ----------

	// Remember who was a neighbor of the node

	ServerPathfindingMessaging::getInstance().sendEraseNode(unloadingNode);

	std::vector<int> oldNeighbors;

	int edgeCount = unloadingNode->getEdgeCount();

	int i;

	for(i = 0; i < edgeCount; i++)
	{
		oldNeighbors.push_back( unloadingNode->getNeighbor(i) );
	}

	// Erase the node

	graph->removeNode(unloadingNode->getIndex());

	// Update the neighbors of the deleted node on the client

	for(i = 0; i < edgeCount; i++)
	{
		CityPathNode * oldNeighbor = graph->_getNode(oldNeighbors[i]);

		if(oldNeighbor) ServerPathfindingMessaging::getInstance().sendNodeInfo( oldNeighbor );
	}

	// If the graph no longer has any nodes in it, destroy it and remove it from the map

	if(graph->getLiveNodeCount() == 0)
	{
		removeFromRegionCache(graph);

		delete graph;
	}
}

// ----------------------------------------------------------------------
//@todo - This is inefficient, but waypoints shouldn't be moving much.

void CityPathGraphManager::moveWaypoint ( ServerObject * sourceObject, Vector const & oldPosition )
{
	UNREF(oldPosition);

	if(sourceObject == nullptr) return;

	CityPathGraph * graph = _getCityGraphFor(sourceObject);

	if(graph == nullptr) return;

	CityPathNode * node = _getCityNodeFor(sourceObject,graph);

	if(node == nullptr) return;

	// ----------

	std::vector<int> oldNeighbors;

	int edgeCount = node->getEdgeCount();

	int i;

	for(i = 0; i < edgeCount; i++)
	{
		oldNeighbors.push_back( node->getNeighbor(i) );
	}

	graph->moveNode( node->getIndex(), sourceObject->getPosition_w() );

	ServerPathfindingMessaging::getInstance().sendNodeInfo( node );
	ServerPathfindingMessaging::getInstance().sendNeighborInfo( node );

	for(i = 0; i < edgeCount; i++)
	{
		CityPathNode * oldNeighbor = graph->_getNode(oldNeighbors[i]);

		if(oldNeighbor) ServerPathfindingMessaging::getInstance().sendNodeInfo( oldNeighbor );
	}
}

// ----------------------------------------------------------------------

void CityPathGraphManager::addBuilding ( BuildingObject * building )
{
	if(building == nullptr) return;

	CityPathGraph * graph = _getCityGraphFor(building);

	if(graph == nullptr) 
	{
		return;
	}

	// ----------

	NetworkIdList ids;
	if (!building->getObjVars().getItem(OBJVAR_PATHFINDING_BUILDING_WAYPOINTS,ids))
	{
		// Building hasn't spawned its waypoints. If we're supposed to do that
		// automatically, do it now.

		if(ConfigSharedPathfinding::getAutoCreateBuildingPathNodes())
		{
			createPathNodes(building);
		}
	}
	else
	{
		// Building has spawned its waypoints. Make sure all the waypoints
		// have their link to this building as their creator

		int idCount = ids.size();

		for(int i = 0; i < idCount; i++)
		{
			ServerObject * serverObject = ServerWorld::findObjectByNetworkId(ids[i]);

			if(serverObject == nullptr) continue;

			CityPathNode * node = _getCityNodeFor(serverObject,graph);

			if(node == nullptr) continue;

			node->setCreator(building->getNetworkId());
		}
	}
}

// ----------------------------------------------------------------------

void CityPathGraphManager::removeBuilding ( BuildingObject * building )
{
	UNREF(building);
}

// ----------------------------------------------------------------------
// When a building is destroyed, we have to (try to) destroy all of its 
// associated path nodes

void CityPathGraphManager::destroyBuilding ( BuildingObject * building )
{
	destroyPathNodes(building);
}

// ----------------------------------------------------------------------

void CityPathGraphManager::moveBuilding ( BuildingObject * building, Vector const & oldPosition )
{
	if(building == nullptr) return;

	CityPathGraph * graph = _getCityGraphFor(building);

	if(graph == nullptr) return;

	UNREF(oldPosition);

	NetworkIdList ids;
	if (!building->getObjVars().getItem(OBJVAR_PATHFINDING_BUILDING_WAYPOINTS,ids)) return;

	int idCount = ids.size();

	for(int i = 0; i < idCount; i++)
	{
		ServerObject * serverObject = ServerWorld::findObjectByNetworkId(ids[i]);

		if(serverObject == nullptr) continue;

		CityPathNode * node = _getCityNodeFor(serverObject,graph);

		if(node == nullptr) continue;

		Vector relativePos_o = node->getRelativePosition_o();

		Vector newPos_w = building->rotateTranslate_o2w( relativePos_o );

		serverObject->setPosition_w(newPos_w);
	}
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::destroyPathGraph ( ServerObject const * creator )
{
	int count = g_regionCache.size();

	for(int i = 0; i < count; i++)
	{
		RegionCacheEntry & entry = g_regionCache[i];

		CityPathGraph * graph = entry.m_graph;
		
		NOT_NULL(graph);

		if(graph->getCreator() == creator)
		{
			ServerPathfindingMessaging::getInstance().sendEraseGraph(graph);

			removeFromRegionCache(graph);

			delete graph;

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------
// Spawn the city path nodes for the given object, and mark the object
// as processed

// Only works for buildings right now.

bool CityPathGraphManager::createPathNodes ( ServerObject * building )
{
	if(building == nullptr) return false;

	// Destroy any old path nodes for the building

	destroyPathNodes(building);

	// ----------
	// Get the path graph for the exterior of the building

	PathGraph const * pathGraph = _getExteriorGraph(building);

	if(pathGraph == nullptr) return false;

	// ----------
	// Go through all the nodes in the building's graph and create city
	// graph nodes for them.

	int nodeCount = pathGraph->getNodeCount();

	NetworkIdList idList;

	AxialBox bounds;

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = pathGraph->getNode(i);

		if(node == nullptr) continue;

		// ----------

		PathNodeType baseType = node->getType();

		PathNodeType newType = baseType;

		if(baseType == PNT_CellPortal)
		{
			newType = PNT_CityBuildingEntrance;
		}
		else
		{
			newType = PNT_CityWaypoint;
		}

		// ----------
		
		Vector position_p = node->getPosition_p();

		Vector position_w = building->rotateTranslate_o2w(position_p);

		bounds.add(position_w);

		Transform tr;
		tr.setPosition_p(position_w);
		ServerObject * newWaypoint = ServerWorld::createNewObject(SERVER_PATHFINDING_WAYPOINT_TEMPLATE_AUTO_SPAWN, tr, 0, true);

		// ServerWorld should handle adding the waypoint to the world, and the notifications will take care of the rest.

		newWaypoint->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_CREATOR,building->getNetworkId());
		newWaypoint->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_KEY, node->getKey() );
		newWaypoint->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE, static_cast<int>(newType) );

		newWaypoint->addToWorld();

		idList.push_back(newWaypoint->getNetworkId());
	}

	// ----------
	// Add a city path node for the building itself. 
	
	// Building nodes have links to all the building entrances, and those links
	// shouldn't change.
	
	{
		Vector position_w = building->getPosition_w();

		bounds.add(position_w);

		Transform tr;
		tr.setPosition_p(position_w);
		ServerObject * newWaypoint = ServerWorld::createNewObject( SERVER_PATHFINDING_WAYPOINT_TEMPLATE_AUTO_SPAWN, tr, 0, true);

		NOT_NULL(newWaypoint);

		newWaypoint->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_CREATOR,building->getNetworkId());
		newWaypoint->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_KEY, static_cast<int>(building->getNetworkId().getHashValue()) );
		newWaypoint->setObjVarItem(OBJVAR_PATHFINDING_WAYPOINT_TYPE, static_cast<int>(PNT_CityBuilding) );

		newWaypoint->addToWorld();

		idList.push_back(newWaypoint->getNetworkId());
	}

	// ----------
	// Done. Mark the building as processed.

	building->setObjVarItem(OBJVAR_PATHFINDING_BUILDING_PROCESSED,1);
	building->setObjVarItem(OBJVAR_PATHFINDING_BUILDING_WAYPOINTS,idList);


	// ----------
	// And mark the chunk of the path graph the building is in as dirty

	{
		CityPathGraph * cityGraph = _getCityGraphFor(building);

		if(cityGraph)
		{
			AxialBox dirty = ShapeUtils::inflate(bounds,getLinkDistance());

			cityGraph->addDirtyBox(dirty);
		}
	}

	
	return true;
}

// ----------------------------------------------------------------------

void CityPathGraphManager::destroyWaypoint ( ServerObject * object )
{
	CityPathGraph * graph = _getCityGraphFor(object);

	if(graph == nullptr) return;

	CityPathNode * deadNode = _getCityNodeFor(object,graph);

	if(deadNode == nullptr) return;

	// ----------

	// Remember who was a neighbor of the node

	ServerPathfindingMessaging::getInstance().sendEraseNode(deadNode);

	std::vector<int> oldNeighbors;

	int edgeCount = deadNode->getEdgeCount();

	int i;

	for(i = 0; i < edgeCount; i++)
	{
		oldNeighbors.push_back( deadNode->getNeighbor(i) );
	}

	// Erase the node

	graph->destroyNode(deadNode->getIndex());

	// Update the neighbors of the deleted node on the client

	for(i = 0; i < edgeCount; i++)
	{
		CityPathNode * oldNeighbor = graph->_getNode(oldNeighbors[i]);

		if(oldNeighbor) ServerPathfindingMessaging::getInstance().sendNodeInfo( oldNeighbor );
	}

	// If the graph no longer has any nodes in it, destroy it and remove it from the map

	if(graph->getLiveNodeCount() == 0)
	{
		removeFromRegionCache(graph);

		delete graph;
	}
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::destroyPathNodes ( ServerObject * object )
{
	if(object == nullptr) return false;

	BuildingObject * building = dynamic_cast<BuildingObject*>(object);

	if(building == nullptr) return false;

	NetworkIdList ids;
	if (!building->getObjVars().getItem(OBJVAR_PATHFINDING_BUILDING_WAYPOINTS,ids)) return false;

	int idCount = ids.size();

	for(int i = 0; i < idCount; i++)
	{
		NetworkId const & id = ids[i];

		if(!id.isValid()) continue;

		ServerObject * serverObject = ServerWorld::findObjectByNetworkId(id);

		if(serverObject == nullptr) continue;

		serverObject->permanentlyDestroy(DeleteReasons::Replaced);
	}

	// wipe all pathfinding-related objvars off of the building

	building->removeObjVarItem(OBJVAR_PATHFINDING);

	return true;
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::reloadPathNodes ( void )
{
	int count = g_regionCache.size();

	for(int i = 0; i < count; i++)
	{
		g_regionCache[i].m_graph->reloadPathNodes();
	}

	return true;
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::reloadPathNodes ( std::vector< ServerObject * > const & objects )
{
	int objectCount = objects.size();

	if(objects.size() == 0) return false;

	CityPathGraph * graph = _getCityGraphFor(objects[0]);

	if(graph == 0) return false;

	for(int i = 0; i < objectCount; i++)
	{
		CityPathNode * node = _getCityNodeFor(objects[i],graph);

		if(node) node->reload();
	}

	return true;
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::markCityEntrance ( ServerObject * object )
{
	CityPathNode * node = _getCityNodeFor(object);

	if(node && (node->getType() == PNT_CityWaypoint))
	{
		node->setType(PNT_CityEntrance);
		node->saveInfoToObjvars();
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

bool CityPathGraphManager::unmarkCityEntrance ( ServerObject * object )
{
	CityPathNode * node = _getCityNodeFor(object);

	if(node && (node->getType() == PNT_CityEntrance))
	{
		node->setType(PNT_CityWaypoint);
		node->saveInfoToObjvars();
		return true;
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------

int CityPathGraphManager::getGraphCount ( void )
{
	return g_regionCache.size();
}

CityPathGraph * CityPathGraphManager::getGraph ( int index )
{
	return g_regionCache[index].m_graph;
}

// ----------------------------------------------------------------------

float CityPathGraphManager::getLinkDistance ( void )
{
	return CityPathGraph::getLinkDistance();
}

void CityPathGraphManager::setLinkDistance ( float dist )
{
	CityPathGraph::setLinkDistance(dist);
}

// ----------------------------------------------------------------------

void CityPathGraphManager::relinkGraph ( CityPathGraph const * constGraph )
{
	if(constGraph == nullptr) return;

	CityPathGraph * graph = const_cast<CityPathGraph*>(constGraph);

	int nodeCount = graph->getNodeCount();

	int i;

	for(i = 0; i < nodeCount; i++)
	{
		graph->relinkNode(i);
	}

	for(i = 0; i < nodeCount; i++)
	{
		ServerPathfindingMessaging::getInstance().sendNodeInfo(graph->_getNode(i));
	}
}

// ======================================================================
