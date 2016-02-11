// ======================================================================
//
// ServerPathBuilder.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/ServerPathBuilder.h"

#include "serverGame/AiLocation.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"

#include "serverPathfinding/CityPathGraph.h"
#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/CityPathNode.h"
#include "serverPathfinding/ServerPathBuildManager.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/CollisionUtils.h"

#include "sharedObject/CellProperty.h"
#include "sharedObject/PortalProperty.h"
#include "sharedObject/PortalPropertyTemplate.h"

#include "sharedPathfinding/PathSearch.h"
#include "sharedPathfinding/PathGraph.h"

#ifdef _DEBUG

#include "sharedDebug/PerformanceTimer.h"

#endif

const float gs_maxCanMoveDistance = 64.0f;
const float gs_maxCanMoveDistance2 = (64.0f * 64.0f);

// ----------------------------------------------------------------------

int findClosestReachablePathNode_slow( CreatureObject const * creature, PathGraph const * graph )
{
	if(graph == nullptr) return -1;

	Vector creaturePos = creature->getPosition_p();

	static std::vector<PathNode*> results;

	results.clear();

	graph->findNodesInRange( creaturePos, gs_maxCanMoveDistance, results);

	int resultCount = results.size();

	PathNode const * reachableNode = nullptr;
	float reachableDist2 = gs_maxCanMoveDistance2;

	for(int i = 0; i < resultCount; i++)
	{
		PathNode const * node = results[i];

		Vector nodePos = node->getPosition_p();

		float dist = nodePos.magnitudeBetweenSquared(creaturePos);

		if(dist < reachableDist2)
		{
			//@todo - use creature radius

			if(CollisionWorld::canMove(creature,nodePos,0.0f,true,false,false) == CMR_MoveOK)
			{
				reachableNode = node;
				reachableDist2 = dist;
			}
		}
	}

	return reachableNode ? reachableNode->getIndex() : -1;
}

int findClosestReachablePathNode( CreatureObject const * creature, PathGraph const * graph )
{
	if(graph == nullptr) return -1;

	Vector creaturePos = creature->getPosition_p();

	// ----------
	// Check to see if the closest node is reachable.
	// This will usually save us a lot of canMove tests.

	// If there is no closest node, or the closest node is farther away than maxCanMoveDistance,
	// return -2 to indicate that the path is incomplete.

	int closestIndex = graph->findNearestNode(creaturePos);

	if(closestIndex < 0) return -2;

	PathNode const * node = graph->getNode(closestIndex);

	if(node == nullptr) return -2;

	Vector nodePos = node->getPosition_p();
	float dist2 = nodePos.magnitudeBetweenSquared(creaturePos);

	if(dist2 > gs_maxCanMoveDistance2) return -2;

	// ----------

	if(CollisionWorld::canMove(creature,nodePos,0.0f,true,false,false) == CMR_MoveOK)
	{
		return node->getIndex();
	}
	else
	{
		// ----------
		// The closest node is not reachable. Search for one that is.

		return findClosestReachablePathNode_slow(creature,graph);
	}
}

// ----------------------------------------------------------------------

int findClosestReachablePathNode_slow ( CellProperty const * cell, Vector const & goal, float radius, PathGraph const * graph )
{
	// ----------
	// The closest node is not reachable. Search for one that is.

	static std::vector<PathNode*> results;

	results.clear();

	graph->findNodesInRange( goal, gs_maxCanMoveDistance, results);

	int resultCount = results.size();

	PathNode const * reachableNode = nullptr;
	float reachableDist2 = gs_maxCanMoveDistance2;

	for(int i = 0; i < resultCount; i++)
	{
		PathNode const * node = results[i];

		Vector nodePos = node->getPosition_p();

		float dist = nodePos.magnitudeBetweenSquared(goal);

		if(dist < reachableDist2)
		{
			//@todo - use creature radius

			if(CollisionWorld::canMove(cell,nodePos,goal,radius,true,false,false) == CMR_MoveOK)
			{
				reachableNode = node;
				reachableDist2 = dist;
			}
		}
	}

	return reachableNode ? reachableNode->getIndex() : -1;
}

int findClosestReachablePathNode ( CellProperty const * cell, Vector const & goal, float radius, PathGraph const * graph )
{
	// ----------
	// Check to see if the closest node is reachable.
	// This will usually save us a lot of canMove tests.

	int closestIndex = graph->findNearestNode(goal);

	if(closestIndex < 0) return -2;

	PathNode const * node = graph->getNode(closestIndex);

	if(node == nullptr) return -2;

	Vector nodePos = node->getPosition_p();
	float dist2 = nodePos.magnitudeBetweenSquared(goal);

	if(dist2 >= gs_maxCanMoveDistance2) return -2;

	// ----------

	if(CollisionWorld::canMove(cell,nodePos,goal,radius,true,false,false) == CMR_MoveOK)
	{
		return node->getIndex();
	}

	// ----------

	return findClosestReachablePathNode_slow(cell,goal,radius,graph);
}

// ----------------------------------------------------------------------

PathGraph const * getGraph ( CellProperty const * cell )
{
	if(cell)
		return safe_cast<PathGraph const *>(cell->getPathGraph());
	else
		return nullptr;
}

PathGraph const * getGraph ( PortalProperty const * building )
{
	if(building)
		return safe_cast<PathGraph const *>(building->getPortalPropertyTemplate().getBuildingPathGraph());
	else
		return nullptr;
}

// ----------

int getKey ( CellProperty const * cell )
{
	if(cell && (cell != CellProperty::getWorldCellProperty()))
		return cell->getCellIndex();
	else
		return -1;
}

// ----------

int getKey ( PortalProperty const * building )
{
	if(building)
		return building->getOwner().getNetworkId().getHashValue();
	else
		return -1;
}

// ----------

PortalProperty const * getBuilding ( BuildingObject const * buildingObject )
{
	if(buildingObject)
		return buildingObject->getPortalProperty();
	else
		return nullptr;
}

PortalProperty const * getBuilding ( CreatureObject const * creature )
{
	if(creature == nullptr) return nullptr;

	CellProperty const * cell = creature->getParentCell();

	if(cell)
		return cell->getPortalProperty();
	else
		return nullptr;
}

PortalProperty const * getBuilding ( AiLocation const & loc )
{
	CellProperty const * cell = loc.getCell();

	if(cell)
		return cell->getPortalProperty();
	else
		return nullptr;
}

// ----------

int getIndexFor ( CityPathGraph const * cityGraph, PortalProperty const * building, CreatureObject const * creature )
{
	if(cityGraph && building)
		return cityGraph->findNode(PNT_CityBuilding,getKey(building));
	else if(cityGraph)
		return findClosestReachablePathNode(creature,cityGraph);
	else
		return -1;
}

int getIndexFor ( CityPathGraph const * cityGraph, PortalProperty const * building, AiLocation const & loc )
{
	if(cityGraph && building)
		return cityGraph->findNode(PNT_CityBuilding,getKey(building));
	else if(cityGraph)
		return findClosestReachablePathNode(CellProperty::getWorldCellProperty(),loc.getPosition_p(),loc.getRadius(),cityGraph);
	else
		return -1;
}

// ----------

int getIndexFor ( PortalProperty const * building, PathGraph const * graph, CreatureObject const * creature )
{
	if(building && graph)
		return graph->findNode(PNT_BuildingCell,getKey(creature->getParentCell()));
	else
		return -1;
}


int getIndexFor ( PortalProperty const * building, PathGraph const * graph, AiLocation const & loc)
{
	if(building && graph)
		return graph->findNode(PNT_BuildingCell,getKey(loc.getCell()));
	else
		return -1;
}

// ----------

int getIndexFor ( PortalProperty const * building, PathGraph const * graph, CreatureObject const * creature, int partId )
{
	int cellIndex = getIndexFor(building,graph,creature);

	if(partId < 0) return cellIndex;

	int neighborCount = graph->getEdgeCount(cellIndex);

	for(int i = 0; i < neighborCount; i++)
	{
		PathNode const * neighbor = graph->getNeighbor(cellIndex,i);

		NOT_NULL(neighbor);

		if(neighbor->getType() == PNT_BuildingCellPart)
		{
			if(neighbor->getId() == partId)
			{
				return neighbor->getIndex();
			}
		}
	}

	return -1;
}

// ----------

int getIndexFor ( PortalProperty const * building, PathGraph const * graph, AiLocation const & loc, int partId )
{
	int cellIndex = getIndexFor(building,graph,loc);

	if(partId < 0) return cellIndex;

	int neighborCount = graph->getEdgeCount(cellIndex);

	for(int i = 0; i < neighborCount; i++)
	{
		PathNode const * neighbor = graph->getNeighbor(cellIndex,i);

		NOT_NULL(neighbor);

		if(neighbor->getType() == PNT_BuildingCellPart)
		{
			if(neighbor->getId() == partId)
			{
				return neighbor->getIndex();
			}
		}
	}

	return -1;
}

// ----------

int getIndexFor ( CellProperty const * cell, PathGraph const * graph, CreatureObject const * creature )
{
	if(cell && graph)
		return findClosestReachablePathNode(creature,graph);
	else
		return -1;
}

int getIndexFor ( CellProperty const * cell, PathGraph const * graph, AiLocation const & loc )
{
	if(cell && graph)
		return findClosestReachablePathNode(cell,loc.getPosition_p(),loc.getRadius(),graph);
	else
		return -1;
}

int getIndexFor ( CellProperty const * cell, PathGraph const * graph, AiLocation const & loc, float overrideRadius )
{
	if(cell && graph)
		return findClosestReachablePathNode(cell,loc.getPosition_p(),overrideRadius,graph);
	else
		return -1;
}

// ----------------------------------------------------------------------

ServerPathBuilder::ServerPathBuilder()
: m_creatureCell(nullptr),
  m_creatureCellGraph(nullptr),
  m_creatureCellKey(-1),
  m_creatureCellNodeIndex(-1),
  m_creatureCellPart(-1),
  m_creatureBuilding(nullptr),
  m_creatureBuildingGraph(nullptr),
  m_creatureBuildingKey(-1),
  m_creatureBuildingNodeIndex(-1),
  m_creaturePosition(),
  m_goalCell(nullptr),
  m_goalCellGraph(nullptr),
  m_goalCellKey(-1),
  m_goalCellNodeIndex(-1),
  m_goalCellPart(-1),
  m_goalBuilding(nullptr),
  m_goalBuildingGraph(nullptr),
  m_goalBuildingKey(-1),
  m_goalBuildingNodeIndex(-1),
  m_goalCityGraph(nullptr),
  m_goalCityNodeIndex(-1),
  m_path(new AiPath()),  
  m_async(false),
  m_buildDone(false),
  m_buildFailed(false),
  m_pathIncomplete(false),
  m_enableJitter(false),
  m_cellSearch( new PathSearch() ),
  m_buildingSearch( new PathSearch() ),
  m_citySearch( new PathSearch() ),
  m_queued(false)
{
}

ServerPathBuilder::~ServerPathBuilder()
{
	ServerPathBuildManager::unqueue(this);

	delete m_path;
	m_path = nullptr;

	delete m_cellSearch;
	m_cellSearch = nullptr;

	delete m_buildingSearch;
	m_buildingSearch = nullptr;

	delete m_citySearch;
	m_citySearch = nullptr;
}


// ----------------------------------------------------------------------
// Cell searches

bool ServerPathBuilder::buildPathInternal( CellProperty const * cell, PathGraph const * graph, int indexA, int indexB )
{
	if(indexA < 0) return false;
	if(indexB < 0) return false;

	if(!m_cellSearch->search(graph,indexA,indexB)) return false;

	// ----------

	IndexList const & path = m_cellSearch->getPath();

	int pathLength = path.size();

	for(int i = 0; i < pathLength; i++)
	{
		int nodeIndex = path[i];

		PathNode const * node = graph->getNode(nodeIndex);

		if(node == nullptr) return false;

		addPathNode( cell, node );
	}

	return true;
}

// ----------------------------------------------------------------------
// Building searches

bool ServerPathBuilder::buildPathInternal ( PortalProperty const * building, PathGraph const * graph, int indexA, int indexB )
{
	if(indexA < 0) return false;
	if(indexB < 0) return false;

	if(!m_buildingSearch->search(graph,indexA,indexB)) return false;

	IndexList const & path = m_buildingSearch->getPath();

	// ----------

	PathNode const * nodeA = nullptr;
	PathNode const * nodeB = nullptr;
	PathNode const * nodeC = nullptr;

	int pathLength = path.size();

	for(int i = 0; i < pathLength + 2; i++)
	{
		if(i < pathLength)
		{
			PathNode const * nextNode = graph->getNode(path[i]);

			if(nextNode == nullptr) continue;

			nodeA = nodeB;
			nodeB = nodeC;
			nodeC = nextNode;
		}
		else
		{
			nodeA = nodeB;
			nodeB = nodeC;
			nodeC = nullptr;
		}

		if( nodeB == nullptr ) continue;

		// ----------

		if( nodeB->hasSubgraph() )
		{
			int cellIndex = nodeB->getKey();

			CellProperty const * subobject = building->getCell(cellIndex);

			if(subobject == nullptr) return false;

			PathGraph const * subgraph = getGraph(subobject);

			if(subgraph == nullptr) return false;

			// ----------

			int indexA = m_creatureCellNodeIndex;
			int indexB = m_goalCellNodeIndex;

			if(nodeA) indexA = subgraph->findEntrance( nodeA->getKey() );
			if(nodeC) indexB = subgraph->findEntrance( nodeC->getKey() );

			if(!buildPathInternal(subobject,subgraph,indexA,indexB)) return false;
		}
		else
		{
			if(nodeB->isConcrete())
			{
				addPathNode( CellProperty::getWorldCellProperty(), nodeB );
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------
// City searches

bool ServerPathBuilder::buildPathInternal ( CityPathGraph const * graph, int indexA, int indexB )
{
	if(indexA < 0) return false;
	if(indexB < 0) return false;

	if(!m_citySearch->search(graph,indexA,indexB)) return false;

	return expandPath( graph, m_citySearch->getPath() );
}

// ----------

bool ServerPathBuilder::buildPathInternal ( CityPathGraph const * graph, int indexA, IndexList const & indexBList )
{
	if(indexA < 0) return false;
	if(indexBList.empty()) return false;

	if(!m_citySearch->search(graph,indexA,indexBList)) return false;

	return expandPath( graph, m_citySearch->getPath() );
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::expandPath ( CityPathGraph const * graph, IndexList const & path )
{
	PathNode const * nodeA = nullptr;
	PathNode const * nodeB = nullptr;
	PathNode const * nodeC = nullptr;

	int pathLength = path.size();

	for(int i = 0; i < pathLength + 2; i++)
	{
		if(i < pathLength)
		{
			PathNode const * nextNode = graph->getNode(path[i]);

			if(nextNode == nullptr) continue;

			nodeA = nodeB;
			nodeB = nodeC;
			nodeC = nextNode;
		}
		else
		{
			nodeA = nodeB;
			nodeB = nodeC;
			nodeC = nullptr;
		}

		if( nodeB == nullptr ) continue;

		// ----------

		if( nodeB->hasSubgraph() )
		{
			CityPathNode const * cityNode = safe_cast<CityPathNode const *>(nodeB);

			if(cityNode == nullptr) return false;

			BuildingObject const * buildingObject = safe_cast<BuildingObject const *>(cityNode->getCreatorObject());

			if(buildingObject == nullptr) return false;

			PortalProperty const * subobject = getBuilding(buildingObject);

			if(subobject == nullptr) return false;

			PathGraph const * subgraph = getGraph(subobject);

			if(subgraph == nullptr) return false;

			// ----------

			int indexA = m_creatureBuildingNodeIndex;
			int indexB = m_goalBuildingNodeIndex;

			if(nodeA) indexA = subgraph->findEntrance( nodeA->getKey() );
			if(nodeC) indexB = subgraph->findEntrance( nodeC->getKey() );

			if(!buildPathInternal(subobject,subgraph,indexA,indexB)) return false;
		}
		else
		{
			if(nodeB->isConcrete())
			{
				addPathNode( CellProperty::getWorldCellProperty(), nodeB );
			}
		}
	}

	return true;
}


// ----------------------------------------------------------------------

bool ServerPathBuilder::buildPath_World ( void )
{
	Vector exitPoint(m_creature ? m_creature->getPosition_w() : m_creaturePosition);

	if((m_creatureCityGraph != nullptr) && (m_creatureCityNodeIndex >= 0))
	{
		int indexA = m_creatureCityNodeIndex;
		int indexB = m_creatureCityGraph->findNearestNode(m_goal.getPosition_w());

		if(!buildPathInternal(m_creatureCityGraph,indexA,indexB)) return false;

		if(indexB >= 0)
		{
			PathNode const * exitNode = m_creatureCityGraph->getNode(indexB);

			if(exitNode)
			{
				exitPoint = exitNode->getPosition_p();
			}
		}
	}

	if((m_goalCityGraph != nullptr) && (m_goalCityNodeIndex >= 0))
	{
		int indexA = m_goalCityGraph->findNearestNode(exitPoint);
		int indexB = m_goalCityNodeIndex;

		if(!buildPathInternal(m_goalCityGraph,indexA,indexB)) return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::buildPath_ToGoal ( void )
{
	m_path->clear();

	m_goal.update();

	if(!m_goal.isValid())
	{
		return false;
	}

	CellProperty const * worldCell = CellProperty::getWorldCellProperty();

	// ----------

	m_creatureCell = m_creature->getParentCell();
	m_goalCell = m_goal.getCell();

	if(m_creatureCell == nullptr) m_creatureCell = worldCell;
	if(m_goalCell == nullptr) m_goalCell = worldCell;

	{
		Vector goalPos_p = m_goal.getPosition_p(m_creatureCell);

		Vector creaturePos_p = m_creature->getPosition_p();

		if(goalPos_p.magnitudeBetween(creaturePos_p) < gs_maxCanMoveDistance2)
		{
			if(CollisionWorld::canMove(m_creature,goalPos_p,0.0f,true,false,false) == CMR_MoveOK)
			{
				return true;
			}
		}
	}

	if((m_creatureCell == worldCell) && (m_goalCell == worldCell))
	{
		// Both the creature and the goal are in the world cell, so
		// we can skip some work with looking up their cell and building
		// info.

		m_creatureCellGraph     = nullptr;
		m_creatureCellKey       = -1;
		m_creatureCellNodeIndex = -1;
		m_creatureCellPart      = -1;

		m_goalCellGraph         = nullptr;
		m_goalCellKey           = -1;
		m_goalCellNodeIndex     = -1;
		m_goalCellPart          = -1;

		m_creatureBuilding          = nullptr;
		m_creatureBuildingGraph     = nullptr;
		m_creatureBuildingKey       = -1;
		m_creatureBuildingNodeIndex = -1;

		m_goalBuilding              = nullptr;
		m_goalBuildingGraph         = nullptr;
		m_goalBuildingKey           = -1;
		m_goalBuildingNodeIndex     = -1;
	}
	else
	{
		// Look up cell info

		m_creatureCellGraph     = getGraph(m_creatureCell);
		m_creatureCellKey       = getKey(m_creatureCell);
		m_creatureCellNodeIndex = getIndexFor(m_creatureCell,m_creatureCellGraph,m_creature);

		if(m_creatureCellGraph && (m_creatureCellNodeIndex >= 0))
		{
			m_creatureCellPart  = m_creatureCellGraph->getNode(m_creatureCellNodeIndex)->getPartId();
		}
		else
		{
			m_creatureCellPart  = -1;
		}

		m_goalCellGraph         = getGraph(m_goalCell);
		m_goalCellKey           = getKey(m_goalCell);
		m_goalCellNodeIndex     = getIndexFor(m_goalCell,m_goalCellGraph,m_goal,0.0f);

		if(m_goalCellGraph && (m_goalCellNodeIndex >= 0))
		{
			m_goalCellPart      = m_goalCellGraph->getNode(m_goalCellNodeIndex)->getPartId();
		}
		else
		{
			m_goalCellPart      = -1;
		}

		// If they're in the same cell, start the search in the cell

		if((m_creatureCell == m_goalCell) && (m_creatureCell != nullptr) && (m_creatureCell != CellProperty::getWorldCellProperty()) && (m_creatureCellPart == m_goalCellPart))
		{
			return buildPathInternal(m_creatureCell,m_creatureCellGraph,m_creatureCellNodeIndex,m_goalCellNodeIndex);
		}

		// Look up building info

		m_creatureBuilding          = m_creatureCell->getPortalProperty();
		m_creatureBuildingGraph     = getGraph(m_creatureBuilding);
		m_creatureBuildingKey       = getKey(m_creatureBuilding);
		m_creatureBuildingNodeIndex = getIndexFor(m_creatureBuilding,m_creatureBuildingGraph,m_creature,m_creatureCellPart);

		m_goalBuilding              = m_goalCell->getPortalProperty();
		m_goalBuildingGraph         = getGraph(m_goalBuilding);
		m_goalBuildingKey           = getKey(m_goalBuilding);
		m_goalBuildingNodeIndex     = getIndexFor(m_goalBuilding,m_goalBuildingGraph,m_goal,m_goalCellPart);

		// If they're in the same building, start the search in the building

		if((m_creatureBuilding == m_goalBuilding) && (m_creatureBuilding != nullptr))
		{
			return buildPathInternal(m_creatureBuilding,m_creatureBuildingGraph,m_creatureBuildingNodeIndex,m_goalBuildingNodeIndex);
		}
	}

	// ----------
	// If they're in the same city, start the search in the city

	m_creatureCityGraph     = CityPathGraphManager::getCityGraphFor(m_creature);
	m_creatureCityNodeIndex = getIndexFor(m_creatureCityGraph,m_creatureBuilding,m_creature);

	m_goalCityGraph         = CityPathGraphManager::getCityGraphFor(m_goal.getPosition_w());
	m_goalCityNodeIndex     = getIndexFor(m_goalCityGraph,m_goalBuilding,m_goal);

	if((m_creatureCityGraph == m_goalCityGraph) && (m_creatureCityGraph != nullptr))
	{
		// hack - If a creature in a city region tries to path search but it's not near a path node,
		// make its path search succeed

		if(m_creatureCell == worldCell)
		{
			if((m_creatureCityNodeIndex == -2) && (m_goalCityNodeIndex == -2))
			{
				m_pathIncomplete = true;
				return true;
			}

			if(m_creatureCityNodeIndex == -2)
			{
				m_creatureCityNodeIndex = m_creatureCityGraph->findNearestNode(m_creature->getPosition_p());
			}

			if(m_goalCityNodeIndex == -2)
			{
				m_goalCityNodeIndex = m_goalCityGraph->findNearestNode(m_goal.getPosition_p());
			}
		}

		return buildPathInternal(m_creatureCityGraph,m_creatureCityNodeIndex,m_goalCityNodeIndex);
	}

	// ----------
	// They're not in the same city, but they have to be in the same world.

	return buildPath_World();
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::buildPath_Named ( void )
{
	m_path->clear();

	// ----------

	m_creatureCell              = m_creature->getParentCell();
	m_creatureCellGraph         = getGraph(m_creatureCell);
	m_creatureCellKey           = getKey(m_creatureCell);
	m_creatureCellNodeIndex     = getIndexFor(m_creatureCell,m_creatureCellGraph,m_creature);

	if(m_creatureCellGraph && (m_creatureCellNodeIndex >= 0))
	{
		m_creatureCellPart      = m_creatureCellGraph->getNode(m_creatureCellNodeIndex)->getPartId();
	}
	else
	{
		m_creatureCellPart      = -1;
	}

	m_creatureBuilding          = m_creatureCell->getPortalProperty();
	m_creatureBuildingGraph     = getGraph(m_creatureBuilding);
	m_creatureBuildingKey       = getKey(m_creatureBuilding);
	m_creatureBuildingNodeIndex = getIndexFor(m_creatureBuilding,m_creatureBuildingGraph,m_creature);

	m_creatureCityGraph         = CityPathGraphManager::getCityGraphFor(m_creature);
	m_creatureCityNodeIndex     = getIndexFor(m_creatureCityGraph,m_creatureBuilding,m_creature);

	// ----------

	if(m_creatureCityGraph != nullptr)
	{
		IndexList goalList;

		int nodeCount = m_creatureCityGraph->getNodeCount();

		for(int i = 0; i < nodeCount; i++)
		{
			CityPathNode const * node = m_creatureCityGraph->_getNode(i);

			if(node == nullptr) continue;

			if(node->getName() == m_goalName)
			{
				goalList.push_back(i);
			}
		}

		return buildPathInternal(m_creatureCityGraph,m_creatureCityNodeIndex,goalList);
	}
	else
	{
		return false;
	}
}

// ----------------------------------------------------------------------
// Fine(er)-grained time management isn't in yet - this is an
// all-or-nothing update

extern float pathSearchTime;
extern float pathBuildTime;

void ServerPathBuilder::update ( void )
{
	bool buildOk = false;

	pathSearchTime = 0;

#ifdef _DEBUG

	PerformanceTimer timer;

	timer.start();

#endif

	if(!m_buildDone)
	{
		if(m_goalName.empty())
		{
			buildOk = buildPath_ToGoal();
		}
		else
		{
			buildOk = buildPath_Named();
		}
	}

#ifdef _DEBUG

	timer.stop();

	pathBuildTime = timer.getElapsedTime();

#endif

	m_buildDone = true;

	if(!buildOk)
	{
		m_buildFailed = true;
	}
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::setupBuildPath ( CreatureObject const * creature, AiLocation const & goal )
{
	if(creature == nullptr) return false;
	if(!goal.isValid()) return false;

	m_creature = creature;
	m_goal = goal;
	m_goalName.clear();
	m_buildDone = false;
	m_buildFailed = false;
	m_pathIncomplete = false;

	return true;
}

// ----------

bool ServerPathBuilder::setupBuildPath ( CreatureObject const * creature, Unicode::String const & goalName )
{
	if(creature == nullptr) return false;
	if(goalName.empty()) return false;

	m_creature = creature;
	m_goal = AiLocation();
	m_goalName = goalName;
	m_buildDone = false;
	m_buildFailed = false;
	m_pathIncomplete = false;

	return true;
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::buildPath_Async ( CreatureObject const * creature, AiLocation const & goal )
{
	if(!setupBuildPath(creature,goal)) return false;

	ServerPathBuildManager::queue(this,false);

	return true;
}

// ----------

bool ServerPathBuilder::buildPath_Async ( CreatureObject const * creature, Unicode::String const & goalName )
{
	if(!setupBuildPath(creature,goalName)) return false;

	ServerPathBuildManager::queue(this,false);

	return true;
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::buildPath ( CreatureObject const * creature, AiLocation const & goal )
{
	if(!setupBuildPath(creature,goal)) return false;

	update();

	return !m_buildFailed;
}

// ----------

bool ServerPathBuilder::buildPath ( CreatureObject const * creature, Unicode::String const & goalName )
{
	if(!setupBuildPath(creature,goalName)) return false;

	update();

	return !m_buildFailed;
}

// ----------------------------------------------------------------------

bool ServerPathBuilder::buildWorldPath(AiLocation const &startLocation, AiLocation const &endLocation)
{
	if (!startLocation.isValid() || !endLocation.isValid())
		return false;

	m_creature = 0;
	m_goal = endLocation;
	m_goalName.clear();
	m_buildDone = false;
	m_buildFailed = false;
	m_pathIncomplete = false;

	m_path->clear();

	m_goal.update();

	if (!m_goal.isValid())
		return false;

	CellProperty const * const worldCell = CellProperty::getWorldCellProperty();

	m_creatureCell = worldCell;
	m_goalCell = worldCell;

	// Both the creature and the goal are in the world cell, so
	// we can skip some work with looking up their cell and building
	// info.

	m_creatureCellGraph     = nullptr;
	m_creatureCellKey       = -1;
	m_creatureCellNodeIndex = -1;
	m_creatureCellPart      = -1;

	m_goalCellGraph         = nullptr;
	m_goalCellKey           = -1;
	m_goalCellNodeIndex     = -1;
	m_goalCellPart          = -1;

	m_creatureBuilding          = nullptr;
	m_creatureBuildingGraph     = nullptr;
	m_creatureBuildingKey       = -1;
	m_creatureBuildingNodeIndex = -1;

	m_goalBuilding              = nullptr;
	m_goalBuildingGraph         = nullptr;
	m_goalBuildingKey           = -1;
	m_goalBuildingNodeIndex     = -1;

	// ----------
	// If they're in the same city, start the search in the city

	m_creatureCityGraph     = CityPathGraphManager::getCityGraphFor(startLocation.getPosition_p());
	m_creatureCityNodeIndex = getIndexFor(m_creatureCityGraph, m_creatureBuilding, startLocation);

	m_goalCityGraph         = CityPathGraphManager::getCityGraphFor(m_goal.getPosition_p());
	m_goalCityNodeIndex     = getIndexFor(m_goalCityGraph, m_goalBuilding, m_goal);

	if (m_creatureCityGraph == m_goalCityGraph && m_creatureCityGraph)
	{
		if (m_creatureCityNodeIndex == -2)
			m_creatureCityNodeIndex = m_creatureCityGraph->findNearestNode(startLocation.getPosition_p());
		if (m_goalCityNodeIndex == -2)
			m_goalCityNodeIndex = m_goalCityGraph->findNearestNode(m_goal.getPosition_p());

		return buildPathInternal(m_creatureCityGraph, m_creatureCityNodeIndex, m_goalCityNodeIndex);
	}

	// ----------
	// They're not in the same city, but they have to be in the same world.

	Vector exitPoint = startLocation.getPosition_p();

	if (m_creatureCityGraph && m_creatureCityNodeIndex >= 0)
	{
		int const indexA = m_creatureCityNodeIndex;
		int const indexB = m_creatureCityGraph->findNearestNode(m_goal.getPosition_p());

		if (!buildPathInternal(m_creatureCityGraph, indexA, indexB))
			return false;

		if (indexB >= 0)
		{
			PathNode const * const exitNode = m_creatureCityGraph->getNode(indexB);
			if (exitNode)
				exitPoint = exitNode->getPosition_p();
		}
	}

	if (m_goalCityGraph && m_goalCityNodeIndex >= 0)
	{
		int const indexA = m_goalCityGraph->findNearestNode(exitPoint);
		int const indexB = m_goalCityNodeIndex;

		if (!buildPathInternal(m_goalCityGraph, indexA, indexB))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------

void ServerPathBuilder::addPathNode ( CellProperty const * cell, PathNode const * node )
{
	if(node == nullptr) return;
	if(cell == nullptr) cell = CellProperty::getWorldCellProperty();

	AiLocation loc(cell,node->getPosition_p());

	float dist2 = REAL_MAX;

	if(m_path && !m_path->empty()) dist2 = m_path->back().getPosition_w().magnitudeBetweenSquared(loc.getPosition_w());

	if(dist2 > 0.01)
	{
		// jitter the node if it's a city waypoint

		if(m_enableJitter && (node->getType() == PNT_CityWaypoint))
		{
			Vector offset = Vector::zero;

			do
			{
				offset = Vector( Random::randomReal(-1.0f,1.0f), 0.0f, Random::randomReal(-1.0f,1.0f) );
			}
			while(offset.magnitudeSquared() > 1.0f);

			static float tweakValue = 1.0f;

			offset *= tweakValue;

			loc.setPosition_p( loc.getPosition_p() + offset );
		}

		m_path->push_back(loc);
	}
}


// ----------------------------------------------------------------------

bool ServerPathBuilder::buildPathImmediate(AiLocation const & creature, AiLocation const & goal)
{
	m_creature = nullptr;
	m_goal = goal;
	m_buildDone = false;
	m_buildFailed = false;
	m_pathIncomplete = false;

	m_creatureCell = creature.getCell();
	m_goalCell = m_goal.getCell();
	m_creaturePosition = creature.getPosition_w();

	m_path->clear();
	m_goalName.clear();
	m_goal.update();


	// ----------
	CellProperty const * worldCell = CellProperty::getWorldCellProperty();
	if(m_creatureCell == nullptr) 
		m_creatureCell = worldCell;
	if(m_goalCell == nullptr) 
		m_goalCell = worldCell;


	// Look up cell info
	m_creatureCellGraph     = getGraph(m_creatureCell);
	m_creatureCellKey       = getKey(m_creatureCell);
	m_creatureCellNodeIndex = getIndexFor(m_creatureCell, m_creatureCellGraph, creature);
	
	if(m_creatureCellGraph && (m_creatureCellNodeIndex >= 0))
	{
		m_creatureCellPart  = m_creatureCellGraph->getNode(m_creatureCellNodeIndex)->getPartId();
	}
	else
	{
		m_creatureCellPart  = -1;
	}
	
	m_goalCellGraph         = getGraph(m_goalCell);
	m_goalCellKey           = getKey(m_goalCell);
	m_goalCellNodeIndex     = getIndexFor(m_goalCell, m_goalCellGraph, m_goal, 0.0f);
	
	if(m_goalCellGraph && (m_goalCellNodeIndex >= 0))
	{
		m_goalCellPart      = m_goalCellGraph->getNode(m_goalCellNodeIndex)->getPartId();
	}
	else
	{
		m_goalCellPart      = -1;
	}

	
	// Look up building info
	m_creatureBuilding          = m_creatureCell->getPortalProperty();
	m_creatureBuildingGraph     = getGraph(m_creatureBuilding);
	m_creatureBuildingKey       = getKey(m_creatureBuilding);
	m_creatureBuildingNodeIndex = getIndexFor(m_creatureBuilding, m_creatureBuildingGraph, creature, m_creatureCellPart);
	
	m_goalBuilding              = m_goalCell->getPortalProperty();
	m_goalBuildingGraph         = getGraph(m_goalBuilding);
	m_goalBuildingKey           = getKey(m_goalBuilding);
	m_goalBuildingNodeIndex     = getIndexFor(m_goalBuilding, m_goalBuildingGraph, m_goal, m_goalCellPart);
	
	// If they're in the same building, start the search in the building
	if((m_creatureBuilding == m_goalBuilding) && (m_creatureBuilding != nullptr))
	{
		if (buildPathInternal(m_creatureBuilding, m_creatureBuildingGraph, m_creatureBuildingNodeIndex, m_goalBuildingNodeIndex))
			return true;
	}

	// If they're in the same cell, start the search in the cell
	if((m_creatureCell == m_goalCell) && (m_creatureCell != nullptr) && (m_creatureCell != CellProperty::getWorldCellProperty()) && (m_creatureCellPart == m_goalCellPart))
	{
		if (buildPathInternal(m_creatureCell, m_creatureCellGraph, m_creatureCellNodeIndex, m_goalCellNodeIndex))
			return true;
	}

	// ----------
	// If they're in the same city, start the search in the city
	m_creatureCityGraph     = CityPathGraphManager::getCityGraphFor(m_creaturePosition);
	m_creatureCityNodeIndex = getIndexFor(m_creatureCityGraph,m_creatureBuilding, creature);

	m_goalCityGraph         = CityPathGraphManager::getCityGraphFor(m_goal.getPosition_w());
	m_goalCityNodeIndex     = getIndexFor(m_goalCityGraph,m_goalBuilding,m_goal);

	if((m_creatureCityGraph == m_goalCityGraph) && (m_creatureCityGraph != nullptr))
	{
		// hack - If a creature in a city region tries to path search but it's not near a path node,
		// make its path search succeed

		if(m_creatureCell == worldCell)
		{
			if((m_creatureCityNodeIndex == -2) && (m_goalCityNodeIndex == -2))
			{
				m_pathIncomplete = true;
				return true;
			}

			if(m_creatureCityNodeIndex == -2)
			{
				m_creatureCityNodeIndex = m_creatureCityGraph->findNearestNode(m_creaturePosition);
			}

			if(m_goalCityNodeIndex == -2)
			{
				m_goalCityNodeIndex = m_goalCityGraph->findNearestNode(m_goal.getPosition_w());
			}
		}

		if (buildPathInternal(m_creatureCityGraph,m_creatureCityNodeIndex,m_goalCityNodeIndex))
			return true;
	}

	// ----------
	// They're not in the same city, but they have to be in the same world.
	
	return buildPath_World();
}

