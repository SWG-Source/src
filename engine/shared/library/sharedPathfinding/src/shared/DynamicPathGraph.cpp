// ======================================================================
//
// DynamicPathGraph.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/DynamicPathGraph.h"

#include "sharedCollision/CollisionWorld.h"
#include "sharedObject/CellProperty.h"
#include "sharedPathfinding/DynamicPathNode.h"

DynamicPathGraph::DynamicPathGraph ( PathGraphType type )
: PathGraph(type),
  m_nodeList( new NodeList() ),
  m_dirtyNodes( new IndexList() ),
  m_liveNodeCount(0)
{
}

DynamicPathGraph::~DynamicPathGraph()
{
	clear();

	delete m_nodeList;
	m_nodeList = nullptr;

	delete m_dirtyNodes;
	m_dirtyNodes = nullptr;
}

// ----------

void DynamicPathGraph::clear ( void )
{
	int nodeCount = getNodeCount();

	for(int i = 0; i < nodeCount; i++)
	{
		delete m_nodeList->at(i);
	}

	m_nodeList->clear();
}

// ----------
// PathGraph interface

int DynamicPathGraph::getNodeCount ( void ) const
{
	return m_nodeList->size();
}

int DynamicPathGraph::getLiveNodeCount ( void ) const
{
	return m_liveNodeCount;
}

PathNode * DynamicPathGraph::getNode ( int nodeIndex )
{
	return _getNode(nodeIndex);
}

PathNode const * DynamicPathGraph::getNode ( int nodeIndex ) const
{
	return _getNode(nodeIndex);
}

int DynamicPathGraph::getEdgeCount ( int nodeIndex ) const
{
	DynamicPathNode const * node = _getNode(nodeIndex);

	if(node == nullptr)
	{
		return 0;
	}
	else
	{
		return node->getEdgeCount();
	}
}

PathEdge * DynamicPathGraph::getEdge ( int nodeIndex, int edgeIndex )
{
	DynamicPathNode * node = _getNode(nodeIndex);

	if(node != nullptr)
	{
		return node->getEdge(edgeIndex);
	}
	else
	{
		return nullptr;
	}
}

PathEdge const * DynamicPathGraph::getEdge ( int nodeIndex, int edgeIndex ) const
{
	DynamicPathNode const * node = _getNode(nodeIndex);

	if(node != nullptr)
	{
		return node->getEdge(edgeIndex);
	}
	else
	{
		return nullptr;
	}
}

// ----------------------------------------------------------------------

int DynamicPathGraph::addNode ( DynamicPathNode * newNode )
{
	if(newNode == nullptr) return -1;

	int listSize = m_nodeList->size();

	int nodeIndex = -1;

	if(m_liveNodeCount < listSize)
	{
		for(int i = 0; i < listSize; i++)
		{
			if(m_nodeList->at(i) == nullptr)
			{
				nodeIndex = i;
				break;
			}
		}
	}
	else
	{
		m_nodeList->resize(listSize + 1);
		nodeIndex = listSize;
	}

	FATAL(nodeIndex == -1,("DynamicPathGraph::addNode - Couldn't add the new node to the node list\n"));

	m_nodeList->at(nodeIndex) = newNode;

	newNode->setIndex(nodeIndex);
	newNode->setGraph(this);

	m_liveNodeCount++;

	return nodeIndex;
}

void DynamicPathGraph::removeNode ( int nodeIndex )
{
	if(nodeIndex == -1) return;

	DynamicPathNode * node = _getNode(nodeIndex);

	if(node != nullptr)
	{
		unlinkNode(nodeIndex);

		m_nodeList->at(nodeIndex) = nullptr;

		delete node;

		m_liveNodeCount--;
	}
}

void DynamicPathGraph::moveNode ( int nodeIndex, Vector const & newPosition )
{
	DynamicPathNode * node = _getNode(nodeIndex);

	if(node != nullptr)
	{
		node->setPosition_p(newPosition);

		m_dirtyNodes->push_back(nodeIndex);

		//@todo - HACK - force cleaning after move

		clean();
	}
}

// ----------------------------------------------------------------------

void DynamicPathGraph::update ( void )
{
	if(isDirty())
	{
		clean();
	}
}

// ----------------------------------------------------------------------

bool DynamicPathGraph::isDirty ( void ) const
{
	return !m_dirtyNodes->empty();
}

// ----------

void DynamicPathGraph::clean ( void )
{
	// Clean all dirty nodes

	/* nothing to do here yet */

	// All dirty nodes are now clean but unlinked. Relink them.

	int dirtyCount = m_dirtyNodes->size();

	for(int i = 0; i < dirtyCount; i++)
	{
		int dirtyIndex = m_dirtyNodes->at(i);

		relinkNode(dirtyIndex);
	}

	// Done.

	m_dirtyNodes->clear();
}

// ----------------------------------------------------------------------

void DynamicPathGraph::unlinkNode ( int nodeIndex )
{
	DynamicPathNode * node = _getNode(nodeIndex);

	if(node == nullptr) return;

	// ----------

	int edgeCount = node->getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		int neighborIndex = node->getNeighbor(i);

		DynamicPathNode * neighborNode = _getNode(neighborIndex);

		if(neighborNode) neighborNode->_removeEdge(nodeIndex);
	}

	node->clearEdges();
}

// ----------------------------------------------------------------------

void DynamicPathGraph::relinkNode ( int nodeIndex )
{
	DynamicPathNode * nodeA = _getNode(nodeIndex);

	if(nodeA == nullptr) return;

	// ----------

	unlinkNode(nodeIndex);

	int listSize = m_nodeList->size();

	int i;

	for(i = 0; i < listSize; i++)
	{
		DynamicPathNode * nodeB = _getNode(i);

		if(nodeB == nullptr) continue;
		if(nodeA == nodeB) continue;

		Vector const & posA = nodeA->getPosition_p();
		Vector const & posB = nodeB->getPosition_p();

		// Only try and link nodes that are within 40 meters of each other

		if(posA.magnitudeBetweenSquared(posB) < (40.0f * 40.0f))
		{
			if(CollisionWorld::canMove( CellProperty::getWorldCellProperty(), posA, posB, 0.5f, true, true, false) == CMR_MoveOK)
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

	nodeA->markRedundantEdges();
	nodeA->removeMarkedEdges();

	// this has to be done in two passes, otherwise node A's neighbor list could be changing
	// as we prune edges.

	std::vector<int> neighborList;

	int edgeCount = nodeA->getEdgeCount();

	for(i = 0; i < edgeCount; i++)
	{
		neighborList.push_back( nodeA->getNeighbor(i) );
	}

	for(i = 0; i < edgeCount; i++)
	{
		DynamicPathNode * neighborNode = _getNode(neighborList[i]);

		if(neighborNode != nullptr)
		{
			neighborNode->markRedundantEdges();
			neighborNode->removeMarkedEdges();
		}
	}
}

// ----------------------------------------------------------------------

DynamicPathNode * DynamicPathGraph::_getNode ( int nodeIndex )
{
	if(nodeIndex == -1) return nullptr;

	return m_nodeList->at(nodeIndex);
}

// ----------

DynamicPathNode const * DynamicPathGraph::_getNode ( int nodeIndex ) const
{
	if(nodeIndex == -1) return nullptr;

	return m_nodeList->at(nodeIndex);
}

// ======================================================================

