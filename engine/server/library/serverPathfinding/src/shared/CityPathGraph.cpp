// ======================================================================
//
// CityPathGraph.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverPathfinding/FirstServerPathfinding.h"
#include "serverPathfinding/CityPathGraph.h"

#include "serverPathfinding/CityPathNode.h"
#include "serverPathfinding/ServerPathfindingConstants.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Containment2d.h"

#include "sharedFoundation/DynamicVariableList.h"

#include "sharedMath/AxialBox.h"
#include "sharedMath/SphereTree.h"

#include "sharedObject/CellProperty.h"

#include "sharedPathfinding/ConfigSharedPathfinding.h"

#include "Unicode.h"
#include "UnicodeUtils.h"

#include <map>
#include <vector>

class PathNodeSphereAccessor;

typedef SphereTree<PathNode *, PathNodeSphereAccessor> PathNodeTree;

float g_linkDistance = 40.0f;

// ----------

class PathNodeSphereAccessor: public BaseSphereTreeAccessor<PathNode *, PathNodeSphereAccessor>
{
public:

	static Sphere const getExtent(PathNode const * const node)
	{
		return Sphere(node->getPosition_p(), 0.5f);
	}

	static char const *getDebugName(PathNode const * const node)
	{
		//@todo - make this return the path node name if there is one

		UNREF(node);

		return "pathNode";
	}

};


// ======================================================================

CityPathGraph::CityPathGraph ( int cityToken )
: DynamicPathGraph(PGT_City),
  m_nodeTree( new PathNodeTree ),
  m_namedNodes( new std::map<Unicode::String, std::set<CityPathNode *> >),
  m_token(cityToken),
  m_dirtyBoxes( new BoxList() )
{
}

CityPathGraph::~CityPathGraph()
{
	delete m_namedNodes;
	m_namedNodes = nullptr;

	delete m_nodeTree;
	m_nodeTree = nullptr;

	delete m_dirtyBoxes;
	m_dirtyBoxes = nullptr;
}

// ----------------------------------------------------------------------

int CityPathGraph::addNode ( DynamicPathNode * newNode )
{
	CityPathNode * cityNode = dynamic_cast<CityPathNode *>(newNode);

	NOT_NULL(cityNode);

	int index = DynamicPathGraph::addNode(newNode);

	SpatialHandle * handle = m_nodeTree->addObject( cityNode );
	if (handle != nullptr)
	{
		if (cityNode->getName() != Unicode::emptyString)
		{
			std::map<Unicode::String, std::set<CityPathNode *> >::iterator found = m_namedNodes->find(cityNode->getName());
			if (found == m_namedNodes->end())
				found = m_namedNodes->insert(std::make_pair(cityNode->getName(), std::set<CityPathNode *>())).first;
			found->second.insert(cityNode);
		}
	}

	cityNode->setSpatialHandle( handle );

	// ----------

	bool dirty = ConfigSharedPathfinding::getEnableDirtyBoxes() && isDirty(cityNode->getPosition_p());

	ServerObject const * source = cityNode->getSourceObject();

	if(source && source->getObjVars().hasItem(OBJVAR_PATHFINDING_WAYPOINT_EDGES) && !dirty)
	{
		cityNode->loadEdgesFromObjvars();
	}
	else
	{
		relinkNode(cityNode->getIndex());

		cityNode->saveAllData();
	}

	return index;
}

// ----------------------------------------------------------------------

void CityPathGraph::removeNode ( int nodeIndex )
{
	CityPathNode * cityNode = _getNode(nodeIndex);
	NOT_NULL(cityNode);

	SpatialHandle * handle = cityNode->getSpatialHandle();

	if (m_namedNodes != nullptr)
	{
		std::map<Unicode::String, std::set<CityPathNode *> >::iterator found = m_namedNodes->find(cityNode->getName());
		if (found != m_namedNodes->end())
		{
			found->second.erase(cityNode);
			if (found->second.empty())
				m_namedNodes->erase(found);
		}
	}
	m_nodeTree->removeObject( handle );

	cityNode->setSpatialHandle(nullptr);

	DynamicPathGraph::removeNode(nodeIndex);
}

// ----------------------------------------------------------------------
// When a waypoint is permanently destroyed from a city graph, we need to 
// make sure that all neighboring nodes are relinked and thatthe edge 
// lists on the objvars are updated as well.

void CityPathGraph::destroyNode ( int nodeIndex )
{
	CityPathNode * cityNode = _getNode(nodeIndex);

	if(cityNode == nullptr) return;

	// ----------
	// Remember which nodes were adjacent to the node we're removing

	std::vector<int> neighborList;

	int edgeCount = cityNode->getEdgeCount();

	int i;

	for(i = 0; i < edgeCount; i++)
	{
		int neighborIndex = cityNode->getNeighbor(i);

		neighborList.push_back( neighborIndex );
	}

	// ----------
	// Update the sphere tree

	SpatialHandle * handle = cityNode->getSpatialHandle();

	m_nodeTree->removeObject( handle );

	cityNode->setSpatialHandle(nullptr);

	// ----------
	// Remove the node

	removeNode( nodeIndex );

	// ----------
	// Relink the neighbors and store their new edge lists back into 
	// the objvars. Do this in two passes to ensure that the edges
	// aren't corrupted (relinking a node can change the edge lists
	// of other nodes)

	for(i = 0; i < edgeCount; i++)
	{
		relinkNode(neighborList[i]);
	}

	for(i = 0; i < edgeCount; i++)
	{
		CityPathNode * neighborNode = _getNode(neighborList[i]);

		if(neighborNode) neighborNode->saveEdgesToObjvars();
	}
}

// ----------------------------------------------------------------------

void CityPathGraph::moveNode ( int nodeIndex, Vector const & newPosition )
{
	CityPathNode * cityNode = _getNode(nodeIndex);

	if(cityNode == nullptr) return;

	// ----------
	// Remember which nodes were adjacent to the node we're removing

	std::vector<int> oldNeighborList;

	int oldEdgeCount = cityNode->getEdgeCount();

	int i;

	for(i = 0; i < oldEdgeCount; i++)
	{
		int neighborIndex = cityNode->getNeighbor(i);

		oldNeighborList.push_back( neighborIndex );
	}

	// ----------
	// Update the sphere tree

	m_nodeTree->move(cityNode->getSpatialHandle());

	// ----------
	// Move the node

	DynamicPathGraph::moveNode(nodeIndex,newPosition);

	// ----------
	// Relink the old neighbors and store their new edge lists back into 
	// the objvars. Do this in two passes to ensure that the edges
	// aren't corrupted (relinking a node can change the edge lists
	// of other nodes)

	for(i = 0; i < oldEdgeCount; i++)
	{
		relinkNode(oldNeighborList[i]);
	}

	for(i = 0; i < oldEdgeCount; i++)
	{
		CityPathNode * oldNeighborNode = _getNode(oldNeighborList[i]);

		if(oldNeighborNode) oldNeighborNode->saveEdgesToObjvars();
	}

	cityNode->saveAllData();
}

// ----------------------------------------------------------------------

void CityPathGraph::relinkNode ( int nodeIndex )
{
	CityPathNode * nodeA = _getNode(nodeIndex);

	if(nodeA == nullptr) return;

	// ----------

	//int oldNeighborCode = getNeighborCode(nodeIndex);

	unlinkNode(nodeIndex);

	PathNodeType typeA = nodeA->getType();

	// ----------
	// Building nodes always connect only to their entrances

	if(typeA == PNT_CityBuilding)
	{
		int listSize = getNodeCount();

		for(int i = 0; i < listSize; i++)
		{
			CityPathNode * nodeB = _getNode(i);

			if(nodeB == nullptr) continue;
			if(nodeA == nodeB) continue;

			PathNodeType typeB = nodeB->getType();

			if(typeB != PNT_CityBuildingEntrance) continue;

			if(nodeA->getCreatorObject() == nullptr) continue;

			if(nodeB->getCreatorObject() == nullptr) continue;

			if(nodeA->getCreatorObject() != nodeB->getCreatorObject()) continue;

			// ----------
			// A is a building, B is an entrance, and they have the same creator

			nodeA->addEdge( nodeB->getIndex() );
			nodeB->addEdge( nodeA->getIndex() );
		}

		// return so we don't try and connect to any other nodes

		return;
	}

	// ----------
	// Building entrances _must_ connect to their building, and may connect
	// to other nodes as well

	if(typeA == PNT_CityBuildingEntrance)
	{
		int listSize = getNodeCount();

		for(int i = 0; i < listSize; i++)
		{
			CityPathNode * nodeB = _getNode(i);

			if(nodeB == nullptr) continue;
			if(nodeA == nodeB) continue;

			PathNodeType typeB = nodeB->getType();

			if(typeB != PNT_CityBuilding) continue;

			if(nodeA->getCreatorObject() == nullptr) continue;

			if(nodeB->getCreatorObject() == nullptr) continue;

			if(nodeA->getCreatorObject() != nodeB->getCreatorObject()) continue;

			// ----------
			// A is an entrance, B is a building, and they have the same creator

			nodeA->addEdge( nodeB->getIndex() );
			nodeB->addEdge( nodeA->getIndex() );
		}
	}

	// ----------

	static std::vector<PathNode*> results;

	results.clear();

	m_nodeTree->findInRange( nodeA->getPosition_p(), g_linkDistance, results );

	int resultCount = results.size();

	int i;

	for(i = 0; i < resultCount; i++)
	{
		CityPathNode * nodeB = static_cast<CityPathNode*>(results[i]);

		if(nodeB == nullptr) continue;
		if(nodeA == nodeB) continue;

		PathNodeType typeB = nodeB->getType();

		if(typeB == PNT_CityBuilding) continue;

		Vector posA = nodeA->getPosition_p();
		Vector posB = nodeB->getPosition_p();

		if(posA.magnitudeBetweenSquared(posB) < (g_linkDistance * g_linkDistance))
		{
			bool moveAB = (CollisionWorld::canMove( CellProperty::getWorldCellProperty(), posA, posB, 0.5f, true, true, false ) == CMR_MoveOK);
			bool moveBA = (CollisionWorld::canMove( CellProperty::getWorldCellProperty(), posB, posA, 0.5f, true, true, false ) == CMR_MoveOK);

			if(moveAB && moveBA)
			{
				nodeA->addEdge( nodeB->getIndex() );
				nodeB->addEdge( nodeA->getIndex() );
			}
			else
			{
				// nodes can't be connected

				continue;
			}
		}
	}

	// ----------
	// Linking done, prune redundant edges

	IGNORE_RETURN( nodeA->markRedundantEdges() );
	IGNORE_RETURN( nodeA->removeMarkedEdges() );

	// this has to be done in two passes, otherwise node A's neighbor list could be changing
	// as we prune edges.

	static std::vector<int> neighborList;

	neighborList.clear();

	int edgeCount = nodeA->getEdgeCount();

	for(i = 0; i < edgeCount; i++)
	{
		neighborList.push_back( nodeA->getNeighbor(i) );
	}

	for(i = 0; i < edgeCount; i++)
	{
		CityPathNode * neighborNode = _getNode(neighborList[i]);

		if(neighborNode != nullptr)
		{
			neighborNode->markRedundantEdges();
			neighborNode->removeMarkedEdges();
		}
	}

	// ----------

	/*
	int newNeighborCode = getNeighborCode(nodeIndex);

	if(oldNeighborCode == newNeighborCode)
	{
		DEBUG_REPORT_LOG(true,("neighbors were the same\n"));
	}
	else
	{
		DEBUG_REPORT_LOG(true,("neighbors were different\n"));
	}
	*/
}

// ----------------------------------------------------------------------

CityPathNode * CityPathGraph::_getNode ( int whichNode )
{
	return safe_cast<CityPathNode *>(getNode(whichNode));
}

// ----------

CityPathNode const * CityPathGraph::_getNode ( int whichNode ) const
{
	return safe_cast<CityPathNode const *>(getNode(whichNode));
}

// ----------------------------------------------------------------------

CityPathNode * CityPathGraph::findNodeForObject ( ServerObject const & object )
{
	int nodeCount = getNodeCount();

	for (int i = 0; i < nodeCount; i++)
	{
		CityPathNode * node = _getNode(i);

		if(node == nullptr) continue;

		ServerObject const * source = node->getSourceObject();

		if(source == nullptr) continue;

		if(source == &object)
		{
			return node;
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

CityPathNode const * CityPathGraph::findNodeForObject ( ServerObject const & object ) const
{
	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		CityPathNode const * node = _getNode(i);

		if(node == nullptr) continue;

		ServerObject const * source = node->getSourceObject();

		if(source == nullptr) continue;

		if(source == &object)
		{
			return node;
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

CityPathNode * CityPathGraph::findNearestNodeForName( Unicode::String const & nodeName, Vector const & pos )
{
	if (m_namedNodes == nullptr)
		return nullptr;

	std::map<Unicode::String, std::set<CityPathNode *> >::iterator found = m_namedNodes->find(nodeName);
	if (found != m_namedNodes->end() && !found->second.empty())
	{
		std::set<CityPathNode *> & nodes = found->second;
		if (nodes.size() == 1)
			return *(nodes.begin());
		
		CityPathNode * node = nullptr;
		float distance = FLT_MAX;
		for (std::set<CityPathNode *>::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			float testDistance = pos.magnitudeBetweenSquared((*i)->getPosition_w());
			if (node == nullptr || testDistance < distance)
			{
				distance = testDistance;
				node = *i;
			}
		}
		return node;
	}

	return nullptr;
}

// ----------------------------------------------------------------------

CityPathNode const * CityPathGraph::findNearestNodeForName( Unicode::String const & nodeName, Vector const & pos ) const
{
	if (m_namedNodes == nullptr)
		return nullptr;

	std::map<Unicode::String, std::set<CityPathNode *> >::const_iterator found = m_namedNodes->find(nodeName);
	if (found != m_namedNodes->end() && !found->second.empty())
	{
		std::set<CityPathNode *> const & nodes = found->second;
		if (nodes.size() == 1)
			return *(nodes.begin());
		
		CityPathNode const * node = nullptr;
		float distance = FLT_MAX;
		for (std::set<CityPathNode *>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			float testDistance = pos.magnitudeBetweenSquared((*i)->getPosition_w());
			if (node == nullptr || testDistance < distance)
			{
				distance = testDistance;
				node = *i;
			}
		}
		return node;
	}

	return nullptr;
}

// ----------------------------------------------------------------------

int CityPathGraph::findNearestNode ( Vector const & position ) const
{
	PathNode * temp = nullptr;

	float dummy1 = REAL_MAX;
	float dummy2 = REAL_MAX;

	if( m_nodeTree->findClosest(position,REAL_MAX,temp,dummy1,dummy2) )
	{
		return temp->getIndex();
	}
	else
	{
		return -1;
	}
}

//@todo - I have no idea why, but the compiler bitches if this isn't here.

int CityPathGraph::findNearestNode ( PathNodeType type, Vector const & position_p ) const
{
	return PathGraph::findNearestNode(type,position_p);
}

// ----------------------------------------------------------------------

void CityPathGraph::findNodesInRange  ( Vector const & position_p, float range, PathNodeList & results ) const
{
	m_nodeTree->findInRange(position_p,range,results);
}

// ----------------------------------------------------------------------

void CityPathGraph::saveGraph ( void )
{
	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		CityPathNode * node = _getNode(i);

		if(node) node->saveToObjvars();
	}
}

// ----------------------------------------------------------------------

bool CityPathGraph::sanityCheck ( bool doWarnings ) const
{
	bool sane = true;

	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		CityPathNode const * node = _getNode(i);

		if(node) 
		{
			sane &= node->sanityCheck(doWarnings);
		}
	}

	return sane;
}

// ----------------------------------------------------------------------

void CityPathGraph::reloadPathNodes ( void )
{
	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		CityPathNode * node = _getNode(i);

		if(node) node->reload();
	}
}

// ----------------------------------------------------------------------

ServerObject const * CityPathGraph::getCreator ( void ) const
{
	return safe_cast<ServerObject const *>(m_creatorId.getObject());
}

void CityPathGraph::setCreator ( NetworkId const & creatorId )
{
	m_creatorId = creatorId;
}

// ----------------------------------------------------------------------

void CityPathGraph::addDirtyBox ( AxialBox const & box )
{
	if(ConfigSharedPathfinding::getEnableDirtyBoxes())
	{
		m_dirtyBoxes->push_back(box);
	}
}

// ----------

bool CityPathGraph::isDirty ( Vector const & point_w ) const
{
	int count = m_dirtyBoxes->size();

	for(int i = 0; i < count; i++)
	{
		AxialBox const & box = m_dirtyBoxes->at(i);

		if(Containment2d::TestPointABox(point_w,box))
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------

float CityPathGraph::getLinkDistance ( void )
{
	return g_linkDistance;
}

void CityPathGraph::setLinkDistance ( float dist )
{
	g_linkDistance = dist;
}

// ----------------------------------------------------------------------
// the neighbor code has to be order-independent, so to make the code
// we cast the addresses of all the neighbors to ints, multiply them
// by a random large prime number, then xor the bits together.

int CityPathGraph::getNeighborCode ( int whichNode ) const
{
	CityPathNode const * node = _getNode(whichNode);

	if(node == nullptr) return 0;

	int edgeCount = node->getEdgeCount();

	int code = 0;

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborId = node->getNeighbor(i);

		CityPathNode const * neighbor = _getNode(neighborId);

		if(neighbor == nullptr) continue;

		int neighborInt = reinterpret_cast<int>(neighbor);

		int mungedInt = neighborInt * 1295183;

		code ^= mungedInt;
	}

	return code;
}

// ----------------------------------------------------------------------
