// ======================================================================
//
// PathGraph.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_PathGraph_H
#define	INCLUDED_PathGraph_H

#include "sharedCollision/BaseClass.h"            // this really needs to be in a different library
#include "sharedPathfinding/PathfindingEnums.h"

class PathGraph;
class PathEdge;
class PathNode;
class Iff;
class DebugShapeRenderer;
class Vector;

typedef std::vector<int> IndexList;
typedef std::vector<PathNode *> PathNodeList;

// ======================================================================

class PathGraph : public BaseClass
{
public:

	PathGraph ( PathGraphType type = PGT_None );
	
	virtual ~PathGraph();

	// ----------

	virtual PathGraphType    getType         ( void ) const;
	virtual void             setType         ( PathGraphType newType );

	virtual int              getNodeCount    ( void ) const = 0;
	virtual PathNode *       getNode         ( int nodeIndex ) = 0;
	virtual PathNode const * getNode         ( int nodeIndex ) const = 0;

	virtual int              getEdgeCount    ( int nodeIndex ) const = 0;
	virtual PathEdge *       getEdge         ( int nodeIndex, int edgeIndex ) = 0;
	virtual PathEdge const * getEdge         ( int nodeIndex, int edgeIndex ) const = 0;

	virtual PathNode *       getNeighbor     ( int nodeIndex, int edgeIndex );
	virtual PathNode const * getNeighbor     ( int nodeIndex, int edgeIndex ) const;

	virtual void             drawDebugShapes ( DebugShapeRenderer * renderer ) const;

	// ----------

	virtual int              findNode        ( PathNodeType type, int key ) const;
	virtual int              findEntrance    ( int entranceKey ) const;
	virtual int              findNearestNode ( Vector const & position_p ) const;
	virtual int              findNearestNode ( PathNodeType type, Vector const & position_p ) const;
	virtual void             findNodesInRange( Vector const & position_p, float range, PathNodeList & results) const;

	virtual int              getNodeKey      ( int nodeIndex ) const;

	// If the A* search is using node marks to associate search nodes with path nodes,
	// then we can only have 1 search going at a time. Locking and unlocking the path
	// graph helps to catch if we try to do more than one search at a time.

	virtual void             searchLock      ( void ) const;
	virtual void             searchUnlock    ( void ) const;

	virtual void             setPartTags     ( void );
	virtual int              getPartCount    ( void ) const;

protected:

	PathGraphType m_type;

	int m_partCount;

	mutable bool m_searchLock;
};

// [1] - The first set of functions allows access to the full set of
// edges in the graph, the second set allows access to the outgoing
// edges for a particular node in the graph.

// ----------

inline PathGraphType PathGraph::getType ( void ) const
{
	return m_type;
}

inline void PathGraph::setType ( PathGraphType newType )
{
	m_type = newType;
}

// ======================================================================

#endif

