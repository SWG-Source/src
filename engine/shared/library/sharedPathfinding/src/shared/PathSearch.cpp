// ======================================================================
//
// PathSearch.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/PathSearch.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"

#include "sharedDebug/PerformanceTimer.h"

#include "sharedPathfinding/PathGraph.h"
#include "sharedPathfinding/PathNode.h"
#include "sharedPathfinding/PathEdge.h"

#include <vector>
#include <algorithm>

const float BIG_HEURISTIC = 1000000000.0f;

// ======================================================================

size_t PathNodeHasher::operator() ( PathNode const * node ) const
{
	return reinterpret_cast<size_t>(node);
}

// ----------------------------------------------------------------------

class PathSearchNode
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	PathSearchNode( PathSearch * search, PathGraph const * graph, PathNode const * node );

	int getNeighborCount ( void );

	PathNode const * getPathNode ( void ) const
	{
		return m_node;
	}

	int getPathNodeIndex ( void ) const
	{
		return m_node->getIndex();
	}

	PathSearchNode * getNeighbor ( int whichNeighbor );

	// ----------

	PathSearchNode * getParent ( void ) const
	{
		return m_parent;
	}

	void setParent ( PathSearchNode * newParent )
	{
		m_parent = newParent;
	}

	// ----------

	float getCost ( void ) const
	{
		return m_cost;
	}

	void setCost   ( float newCost )
	{
		m_cost = newCost;
		m_total = m_cost + m_heuristic;
	}

	float getTotal ( void ) const
	{
		return m_total;
	}

	// ----------

	bool isQueued ( void )
	{
		return m_queued;
	}

	void setQueued ( bool queued )
	{
		m_queued = queued;
	}

	// ----------

	void setPathIndex ( int index )
	{
		m_pathIndex = index;
	}

	int getPathIndex ( void ) const
	{
		return m_pathIndex;
	}

	// ----------

	PathSearchNode * getSearchNode    ( PathNode const * node );
	PathSearchNode * createSearchNode ( PathNode const * node );

	// ----------

protected:

	PathSearch *      m_search;
	PathSearchNode *  m_parent;
	PathGraph const * m_graph;
	PathNode const *  m_node;

	bool   m_queued;
	
	float  m_cost;
	float  m_heuristic;
	float  m_total;

	int    m_pathIndex;
};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(PathSearchNode, true, 0, 0, 0);

// ----------

PathSearchNode::PathSearchNode ( PathSearch * search, PathGraph const * graph, PathNode const * node )
: m_search(search),
  m_parent(nullptr),
  m_graph(graph),
  m_node(node),
  m_queued(false),
  m_cost(REAL_MAX),
  m_heuristic(0.0f),
  m_total(REAL_MAX),
  m_pathIndex(-1)
{
	m_heuristic = m_search->calcHeuristic(m_node);
}

// ----------------------------------------------------------------------

int PathSearchNode::getNeighborCount ( void )
{
	return m_graph->getEdgeCount( m_node->getIndex() );
}

// ----------

PathSearchNode * PathSearchNode::getNeighbor ( int whichNeighbor )
{
	int neighborIndex = m_graph->getEdge( m_node->getIndex(), whichNeighbor )->getIndexB();

	PathNode const * neighborNode = m_graph->getNode(neighborIndex);

	if(neighborNode != nullptr)
	{
		return getSearchNode(neighborNode);
	}
	else
	{
		return nullptr;
	}
}

// ----------------------------------------------------------------------

PathSearchNode * PathSearchNode::createSearchNode( PathNode const * node )
{
	// doing bad voodoo with the node marks

	PathSearchNode * oldNode = nullptr;

	int mark = node->getMark(3);

	if(mark != -1)
	{
		oldNode = (PathSearchNode*)((void*)mark);
	}

	delete oldNode;

	PathSearchNode * searchNode = new PathSearchNode(m_search,m_graph,node);

	node->setMark( 3, (int)((void*)searchNode) );

	m_search->m_visitedNodes->push_back(node);

	return searchNode;
}

// ----------

PathSearchNode * PathSearchNode::getSearchNode( PathNode const * node )
{
	// doing bad voodoo with the node marks

	PathSearchNode * searchNode = nullptr;

	int mark = node->getMark(3);

	if(mark != -1)
	{
		searchNode = (PathSearchNode*)((void*)mark);
	}

	if(searchNode == nullptr)
	{
		return createSearchNode(node);
	}
	else
	{
		return searchNode;
	}
}

// ======================================================================

bool PathSearchGreater( PathSearchNode const * A, PathSearchNode const * B )
{
	return A->getTotal() > B->getTotal();
}

// ======================================================================

class PathSearchQueue
{
public:

	void push ( PathSearchNode * node )
	{
		m_nodes.push_back(node);

		std::push_heap(m_nodes.begin(),m_nodes.end(),PathSearchGreater);

		node->setQueued(true);
	}

	PathSearchNode * pop ( void )
	{
		PathSearchNode * node = m_nodes.front();

		std::pop_heap(m_nodes.begin(),m_nodes.end(),PathSearchGreater);
		m_nodes.pop_back();

		node->setQueued(false);

		return node;
	}

	void update ( PathSearchNode * node )
	{
		if(node->isQueued())
		{
			for(uint i = 0; i < m_nodes.size(); i++)
			{
				if(m_nodes[i] == node)
				{
					std::push_heap( m_nodes.begin(), m_nodes.begin() + i + 1, PathSearchGreater );
					return;
				}
			}
		}
		else
		{
			push(node);
		}
	}

	void clear ( void )
	{
		m_nodes.clear();
	}

	bool empty ( void )
	{
		return m_nodes.empty();
	}

	// ----------

protected:

	std::vector<PathSearchNode *> m_nodes;
};

// ======================================================================

void PathSearch::install()
{
	PathSearchNode::install();
}

// ----------------------------------------------------------------------

PathSearch::PathSearch ( void )
: m_graph(nullptr),
  m_start(nullptr),
  m_goal(nullptr),
  m_multiGoal(false),
  m_goals(new NodeList()),
  m_queue(new PathSearchQueue()),
  m_path(new IndexList()),
  m_visitedNodes(new NodeList())
{
	m_path->reserve(20);
	m_visitedNodes->reserve(20);
}

PathSearch::~PathSearch()
{
	delete m_goals;
	m_goals = nullptr;

	delete m_queue;
	m_queue = nullptr;

	delete m_path;
	m_path = nullptr;

	delete m_visitedNodes;
	m_visitedNodes = nullptr;
}

// ----------------------------------------------------------------------

PathSearchNode * PathSearch::search ( void )
{
	PathSearchNode * startNode = new PathSearchNode(this,m_graph,m_start);

	startNode->setCost(0.0f);
	startNode->setPathIndex(0);

	m_queue->push(startNode);

	while(!m_queue->empty())
	{
		PathSearchNode * node = m_queue->pop();

		if(atGoal(node))
		{
			return node;
		}

		// ----------

		int neighborCount = node->getNeighborCount();

		for(int i = 0; i < neighborCount; i++)
		{
			PathSearchNode * neighbor = node->getNeighbor(i);

			if(neighbor != nullptr)
			{
				float newCost = node->getCost() + costBetween(node->getPathNode(),neighbor->getPathNode());

				if( newCost < neighbor->getCost() )
				{
					neighbor->setParent(node);
					neighbor->setCost(newCost);
					neighbor->setPathIndex( node->getPathIndex() + 1);
					
					m_queue->update(neighbor);
				}
			}
		}
	}

	return nullptr;
}

// ----------------------------------------------------------------------

extern float pathSearchTime;

bool PathSearch::search ( PathGraph const * graph, int startIndex, int goalIndex )
{
	PerformanceTimer timer;

	timer.start();

	m_graph = graph;
	m_start = graph->getNode(startIndex);
	m_goal = graph->getNode(goalIndex);
	m_multiGoal = false;

	if(m_start == nullptr)	return false;
	if(m_goal == nullptr) return false;

	m_path->clear();

	PathSearchNode * endNode = search();

	bool buildOk = buildPath(endNode);

	cleanup();

	timer.stop();

	pathSearchTime += timer.getElapsedTime();

	return buildOk;
}

// ----------------------------------------------------------------------

bool PathSearch::search ( PathGraph const * graph, int startIndex, IndexList const & goalIndices )
{
	PerformanceTimer timer;

	timer.start();

	m_graph = graph;
	m_start = graph->getNode(startIndex);
	
	m_multiGoal = true;

	int goalCount = goalIndices.size();

	if(goalCount == 0) return false;
	if(m_start == nullptr)	return false;

	m_goals->resize(goalCount);

	for(int i = 0; i < goalCount; i++)
	{
		m_goals->at(i) = graph->getNode(goalIndices[i]);
	}

	PathSearchNode * endNode = search();

	bool buildOk = buildPath(endNode);

	cleanup();

	timer.stop();

	pathSearchTime += timer.getElapsedTime();

	return buildOk;
}

// ----------------------------------------------------------------------

bool PathSearch::buildPath ( PathSearchNode * endNode )
{
	if( endNode == nullptr )
	{
		m_path->clear();
		return false;
	}

	// ----------

	int pathLength = endNode->getPathIndex() + 1;

	m_path->resize(pathLength);

	PathSearchNode * cursor = endNode;

	while(cursor)
	{
		int pathIndex = cursor->getPathIndex();
		int nodeIndex = cursor->getPathNodeIndex();

		m_path->at(pathIndex) = nodeIndex;

		cursor = cursor->getParent();
	}

	return true;
}


// ----------------------------------------------------------------------

void PathSearch::cleanup ( void )
{
	int visitedCount = m_visitedNodes->size();

	for(int i = 0; i < visitedCount; i++)
	{
		PathNode const * visitedNode = m_visitedNodes->at(i);

		int mark = visitedNode->getMark(3);

		if(mark != -1)
		{
			PathSearchNode * searchNode = (PathSearchNode*)((void*)mark);

			delete searchNode;
		}

		visitedNode->clearMarks();
	}

	m_queue->clear();
	m_goals->clear();
	m_visitedNodes->clear();

	m_multiGoal = false;
}

// ----------------------------------------------------------------------

float PathSearch::costBetween ( PathNode const * A, PathNode const * B ) const
{
	if(m_graph->getType() == PGT_Building)
	{
		if(A->getType() == PNT_BuildingCell)
		{
			return BIG_HEURISTIC;
		}

		if(B->getType() == PNT_BuildingCell)
		{
			return BIG_HEURISTIC;
		}

		return 1.0f;
	}
	else
	{
		Vector posA = A->getPosition_p();
		Vector posB = B->getPosition_p();

		posA.y = 0;
		posB.y = 0;

		float dist = posA.magnitudeBetween(posB);

		return dist;
	}
}

// ----------------------------------------------------------------------

float PathSearch::calcHeuristic ( PathNode const * A ) const
{
	if(!m_graph) return BIG_HEURISTIC;

	if(m_multiGoal)
	{
		int minGoal = -1;
		float minHeuristic = REAL_MAX;

		int goalCount = m_goals->size();

		for(int i = 0; i < goalCount; i++)
		{
			PathNode const * goal = m_goals->at(i);

			float heuristic = calcHeuristic(A,goal);

			if(heuristic < minHeuristic)
			{
				minHeuristic = heuristic;
				minGoal = i;
			}
		}

		if(minGoal != -1)
		{
			return minHeuristic;
		}
		else
		{
			return BIG_HEURISTIC;
		}
	}
	else
	{
		return calcHeuristic(A,m_goal);
	}
}

// ----------------------------------------------------------------------

float PathSearch::calcHeuristic ( PathNode const * A, PathNode const * goal ) const
{
	if(m_graph->getType() == PGT_Building)
	{
		// HACK - Exporter versions 2.136 and earlier created building graph
		// edges connected to cell 0, which is incorrect since cell 0 represents
		// the outside of the building.

		// Get around this by making any edge connected to the graph node
		// for cell 0 have a huge heuristic.

		if(m_goal->getType() == PNT_BuildingCell)
		{
			if(m_goal->getKey() == 0)
			{
				return BIG_HEURISTIC;
			}
		}

		if(A->getType() == PNT_BuildingCell)
		{
			if(A->getKey() == 0)
			{
				return BIG_HEURISTIC;
			}
		}

		return 0;
	}
	else
	{
		return costBetween(A,goal) * 3.0f;
	}
}

// ----------------------------------------------------------------------

IndexList const & PathSearch::getPath ( void ) const
{
	return *m_path;
}

// ----------------------------------------------------------------------

bool PathSearch::atGoal ( PathSearchNode * searchNode ) const
{
	if(searchNode == nullptr) return false;

	if(m_multiGoal)
	{
		PathNode const * pathNode = searchNode->getPathNode();

		if(pathNode == nullptr) return false;

		return std::find( m_goals->begin(), m_goals->end(), pathNode ) != m_goals->end();
	}
	else
	{
		return m_goal && (m_goal == searchNode->getPathNode());
	}
}

// ----------------------------------------------------------------------
