// ======================================================================
//
// PathGraph.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/PathGraph.h"

#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/PathEdge.h"

#include <vector>
#include <stack>

typedef std::stack<PathNode *, std::vector<PathNode *> > PathNodeStack;

// ======================================================================

PathGraph::PathGraph ( PathGraphType type )
: m_type(type),
  m_partCount(-1),
  m_searchLock(false)
{
}

PathGraph::~PathGraph()
{
}

// ----------------------------------------------------------------------

PathNode * PathGraph::getNeighbor ( int nodeIndex, int edgeIndex )
{
	PathEdge * edge = getEdge(nodeIndex,edgeIndex);

	int neighborIndex = edge->getIndexB();

	return getNode(neighborIndex);
}

// ----------

PathNode const * PathGraph::getNeighbor ( int nodeIndex, int edgeIndex ) const
{
	PathEdge const * edge = getEdge(nodeIndex,edgeIndex);

	int neighborIndex = edge->getIndexB();

	return getNode(neighborIndex);
}

// ----------------------------------------------------------------------

void PathGraph::drawDebugShapes ( DebugShapeRenderer * ) const
{
}

// ----------------------------------------------------------------------

int PathGraph::findNode ( PathNodeType type, int key ) const
{
	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = getNode(i);

		if(getEdgeCount(i) == 0) continue;
		
		if((node != nullptr) && (node->getType() == type) && (node->getKey() == key))
		{
			return i;
		}
	}

	return -1;
}

// ----------------------------------------------------------------------

int PathGraph::findEntrance ( int key ) const
{
	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = getNode(i);

		if(node == nullptr) continue;
		
		if(getEdgeCount(i) == 0) continue;
		
		PathNodeType type = node->getType();

		if((type == PNT_CellPortal) || (type == PNT_BuildingEntrance) || (type == PNT_CityEntrance))
		{
			if(node->getKey() == key)
			{
				return i;
			}
		}
	}

	return -1;
}

// ----------------------------------------------------------------------

int PathGraph::findNearestNode ( Vector const & position_p ) const
{
	int minNode = -1;
	float minDist2 = REAL_MAX;

	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = getNode(i);

		if(node == nullptr) continue;

		if(getEdgeCount(i) == 0) continue;
		
		float dist2 = position_p.magnitudeBetweenSquared(node->getPosition_p());

		if(dist2 < minDist2)
		{
			minNode = i;
			minDist2 = dist2;
		}
	}

	return minNode;
}

// ----------------------------------------------------------------------

int PathGraph::findNearestNode ( PathNodeType searchType, Vector const & position_p ) const
{
	int minNode = -1;
	float minDist2 = REAL_MAX;

	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = getNode(i);

		if(node == nullptr) continue;
		
		if(getEdgeCount(i) == 0) continue;
		
		PathNodeType type = node->getType();

		if(type != searchType) continue;

		float dist2 = position_p.magnitudeBetweenSquared(node->getPosition_p());

		if(dist2 < minDist2)
		{
			minNode = i;
			minDist2 = dist2;
		}
	}

	return minNode;
}

// ----------------------------------------------------------------------

void PathGraph::findNodesInRange ( Vector const & position_p, float range, PathNodeList & results ) const
{
	float range2 = range * range;

	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode const * node = getNode(i);

		if(node == nullptr) continue;
		
		if(getEdgeCount(i) == 0) continue;
		
		float dist2 = position_p.magnitudeBetweenSquared(node->getPosition_p());

		if(dist2 < range2)
		{
			results.push_back( const_cast<PathNode*>(node) );
		}
	}
}

// ----------------------------------------------------------------------

int PathGraph::getNodeKey ( int nodeIndex ) const
{
	PathNode const * node = getNode(nodeIndex);

	return node ? node->getKey() : -1;
}

// ----------------------------------------------------------------------

void PathGraph::searchLock ( void ) const
{
	DEBUG_FATAL(m_searchLock,("PathGraph::searchLock - Tried to lock a path graph that was already locked\n"));

	m_searchLock = true;
}

void PathGraph::searchUnlock ( void ) const
{
	DEBUG_WARNING(!m_searchLock,("PathGraph::searchUnlock - Graph was already unlocked\n"));

	m_searchLock = false;
}

// ----------------------------------------------------------------------

void PathGraph::setPartTags ( void )
{
	static PathNodeStack unprocessed;

	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		PathNode * node = getNode(i);

		node->setPartId(-1);

		// ignore nodes without edges unless they're the only node in the cell

		int edgeCount = getEdgeCount(node->getIndex());

		if(edgeCount > 0)
		{
			unprocessed.push(node);
		}
		else
		{
			PathNodeType type = node->getType();

			if((nodeCount == 1) && (type == PNT_CellPortal))
			{
				unprocessed.push(node);
			}
		}
	}

	static PathNodeStack processing;

	int currentTag = 0;

	while(!unprocessed.empty())
	{
		PathNode * currentNode = unprocessed.top();
		unprocessed.pop();

		if(currentNode->getPartId() != -1) continue;

		processing.push(currentNode);

		while(!processing.empty())
		{
			PathNode * node = processing.top();
			processing.pop();

			if(node->getPartId() != -1) continue;

			node->setPartId(currentTag);

			int edgeCount = getEdgeCount(node->getIndex());

			for(int i = 0; i < edgeCount; i++)
			{
				int neighborIndex = getEdge(node->getIndex(),i)->getIndexB();

				PathNode * neighborNode = getNode(neighborIndex);

				processing.push(neighborNode);
			}
		}

		currentTag++;
	}

	m_partCount = currentTag;
}

// ----------

int PathGraph::getPartCount ( void ) const
{
	return m_partCount;
}

// ======================================================================
