// ======================================================================
//
// PathGraph.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/PathGraphIterator.h"

#include "sharedPathfinding/PathGraph.h"
#include "sharedPathfinding/PathEdge.h"

// ======================================================================

PathGraphIterator::PathGraphIterator ()
: m_graph(nullptr),
  m_nodeIndex(-1)
{
}

PathGraphIterator::PathGraphIterator ( PathGraph const * graph, int nodeIndex )
: m_graph(graph),
  m_nodeIndex(nodeIndex)
{
}

// ----------

bool PathGraphIterator::isValid ( void ) const
{
	return (m_graph != nullptr) && (m_nodeIndex != -1) && (m_graph->getNode(m_nodeIndex) != nullptr);
}

PathNode const * PathGraphIterator::getNode ( void ) const
{
	if( (m_graph != nullptr) && (m_nodeIndex != -1) )
	{
		return m_graph->getNode( m_nodeIndex );
	}
	else
	{
		return nullptr;
	}
}

int PathGraphIterator::getNeighborCount ( void ) const
{
	if(!isValid()) return 0;

	return m_graph->getEdgeCount( m_nodeIndex );
}

PathNode const * PathGraphIterator::getNeighbor ( int whichNeighbor ) const
{
	if(!isValid()) return nullptr;

	return m_graph->getNode( m_graph->getEdge( m_nodeIndex, whichNeighbor )->getIndexB() );
}

void PathGraphIterator::stepTo ( int whichNeighbor )
{
	if(!isValid()) return;

	m_nodeIndex = m_graph->getEdge( m_nodeIndex, whichNeighbor )->getIndexB();
}

// ======================================================================
