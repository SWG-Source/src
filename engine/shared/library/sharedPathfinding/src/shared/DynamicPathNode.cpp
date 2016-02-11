// ======================================================================
//
// DynamicPathNode.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/DynamicPathNode.h"

#include "sharedPathfinding/PathEdge.h"
#include "sharedPathfinding/DynamicPathGraph.h"

static float angleBetween ( Vector const & A, Vector const & B, Vector const & C )
{
	Vector dA = (B-A);
	Vector dB = (C-A);

	IGNORE_RETURN(dA.normalize());
	IGNORE_RETURN(dB.normalize());
	
	float dot = dA.dot(dB);

	return acos(dot);
}

// ======================================================================

DynamicPathNode::DynamicPathNode ( void )
: PathNode(),
  m_edges()
{
}

DynamicPathNode::DynamicPathNode ( Vector const & position_p )
: PathNode( position_p ),
  m_edges()
{
}

DynamicPathNode::~DynamicPathNode()
{
}

// ----------------------------------------------------------------------

int DynamicPathNode::getEdgeCount ( void ) const
{
	return m_edges.size();
}

// ----------

PathEdge * DynamicPathNode::getEdge ( int whichEdge )
{
	return &m_edges[whichEdge];
}

// ----------

PathEdge const * DynamicPathNode::getEdge ( int whichEdge ) const
{
	return &m_edges[whichEdge];
}

// ----------

int DynamicPathNode::getNeighbor ( int whichEdge ) const
{
	return m_edges[whichEdge].getIndexB();
}

// ----------------------------------------------------------------------

bool DynamicPathNode::hasEdge ( int nodeIndex ) const
{
	int edgeCount = m_edges.size();

	for(int i = 0; i < edgeCount; i++)
	{
		if(m_edges[i].getIndexB() == nodeIndex)
		{
			return true;
		}
	}

	return false;
}

// ----------

bool DynamicPathNode::addEdge ( int nodeIndex )
{
	if(!hasEdge(nodeIndex))
	{
		m_edges.push_back( PathEdge( m_index, nodeIndex ) );
	}

	return true;
}

// ----------

bool DynamicPathNode::removeEdge ( int nodeIndex )
{
	DynamicPathNode * neighbor = _getGraph()->_getNode(nodeIndex);

	if(neighbor == nullptr) return false;
	
	if(!_removeEdge(nodeIndex)) return false;

	return neighbor->_removeEdge( getIndex() );
}

bool DynamicPathNode::_removeEdge ( int nodeIndex )
{
	int edgeIndex = -1;

	int edgeCount = m_edges.size();

	for(int i = 0; i < edgeCount; i++)
	{
		if(m_edges[i].getIndexB() == nodeIndex)
		{
			edgeIndex = i;
			break;
		}
	}

	if(edgeIndex != -1)
	{
		m_edges.erase( m_edges.begin() + edgeIndex );
		return true;
	}
	else
	{
		return false;
	}
}

// ----------

void DynamicPathNode::clearEdges ( void )
{
	m_edges.clear();
}

// ----------------------------------------------------------------------

void DynamicPathNode::clearEdgeMarks ( void ) const
{
	int edgeCount = getEdgeCount();

	for(int i = 0; i < edgeCount; i++)
	{
		getEdge(i)->clearMarks();
	}
}

// ----------------------------------------------------------------------

int DynamicPathNode::markRedundantEdges ( void ) const
{
	clearEdgeMarks();

	float angleTolerance = (20.0f / 360.0f) * (2.0f * PI);

	int edgeCount = getEdgeCount();
	int markCount = 0;

	for(int i = 0; i < edgeCount - 1; i++)
	{
		for(int j = i+1; j < edgeCount; j++)
		{
			PathEdge const * edgeA = getEdge(i);
			PathEdge const * edgeB = getEdge(j);

			if(edgeA == nullptr) continue;
			if(edgeB == nullptr) continue;

			int iA = edgeA->getIndexA();
			int iB = edgeA->getIndexB();
			int iC = edgeB->getIndexB();

			if(getGraph()->getNode(iA)->getType() == PNT_CityBuilding) continue;
			if(getGraph()->getNode(iB)->getType() == PNT_CityBuilding) continue;
			if(getGraph()->getNode(iC)->getType() == PNT_CityBuilding) continue;

			Vector A = getGraph()->getNode(iA)->getPosition_p();
			Vector B = getGraph()->getNode(iB)->getPosition_p();
			Vector C = getGraph()->getNode(iC)->getPosition_p();

			if(angleBetween(A,B,C) < angleTolerance)
			{
				float magAB = (B-A).magnitudeSquared();
				float magAC = (C-A).magnitudeSquared();

				if(magAB > magAC)
				{
					// The A-B edge is longer, so mark edgeA as redundant.

					if(edgeA->getMark(0) != 1)
					{
						edgeA->setMark(0,1);
						markCount++;
					}
				}
				else
				{
					// The A-C edge is longer, so mark edgeB as redundant.

					if(edgeB->getMark(0) != 1)
					{
						edgeB->setMark(0,1);
						markCount++;
					}
				}
			}
		}
	}

	return markCount;
}

// ----------------------------------------------------------------------
// not the most efficient way to do this, but it should be OK.

int DynamicPathNode::removeMarkedEdges ( void )
{
	int removeCount = 0;

	bool removed = true;

	while(removed)
	{
		removed = false;

		int edgeCount = getEdgeCount();

		for(int i = 0; i < edgeCount; i++)
		{
			PathEdge * edge = getEdge(i);

			if(edge && (edge->getMark(0) != -1))
			{
				if(removeEdge(edge->getIndexB()))
				{
					removeCount++;
					removed = true;
					break;
				}
			}
		}
	}

	return removeCount;
}

// ----------------------------------------------------------------------

DynamicPathGraph * DynamicPathNode::_getGraph ( void )
{
	return safe_cast< DynamicPathGraph * >( getGraph() );
}

// ----------------------------------------------------------------------

