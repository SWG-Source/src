// ======================================================================
//
// SimplePathGraph.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_SimplePathGraph_H
#define	INCLUDED_SimplePathGraph_H

#include "sharedPathfinding/PathGraph.h"

#include "sharedMath/Vector.h"

class FloorLocator;

typedef std::pair<int,int> IndexPair;
typedef std::vector<int> IntList;
typedef std::vector<Vector> VectorList;
typedef std::vector<IndexPair> IndexPairList;
typedef std::vector<FloorLocator> FloorLocatorList;

// ======================================================================
// Very simple concrete path graph class, uses base PathEdge and PathNode
// classes and stores them in a vector.

class SimplePathGraph : public PathGraph
{
public:

	typedef std::vector<PathEdge> EdgeList;
	typedef std::vector<PathNode> NodeList;

	SimplePathGraph ( PathGraphType type = PGT_None );

	SimplePathGraph ( NodeList * nodes, EdgeList * edges, PathGraphType type = PGT_None );
	
	virtual ~SimplePathGraph();

	// ----------

	void clear ( void );

	// ----------
	// PathGraph interface

	virtual int              getNodeCount    ( void ) const;
	virtual PathNode *       getNode         ( int nodeIndex );
	virtual PathNode const * getNode         ( int nodeIndex ) const;

	virtual int              getEdgeCount    ( int nodeIndex ) const;
	virtual PathEdge *       getEdge         ( int nodeIndex, int edgeIndex );
	virtual PathEdge const * getEdge         ( int nodeIndex, int edgeIndex ) const;

	virtual void             read            ( Iff & iff );
	virtual void             write           ( Iff & iff ) const;

	virtual void             drawDebugShapes ( DebugShapeRenderer * renderer ) const;

	// ----------
	// SimplePathGraph interface

	virtual NodeList const & getNodes        ( void ) const;
	virtual EdgeList const & getEdges        ( void ) const;

	// ----------

	int  getVersion ( void ) const;

	void read_old   ( Iff & iff );

protected:

	void buildIndexTables ( void );

	void read_0000        ( Iff & iff );
	void read_0001        ( Iff & iff );

	// ----------

	int          m_version;

	NodeList *   m_nodes;
	EdgeList *   m_edges;
	IndexList *  m_edgeCounts;
	IndexList *  m_edgeStarts;

#ifdef _DEBUG

	void buildDebugData ( void );

	VectorList * m_debugLines;

#endif
};

// ----------

inline int SimplePathGraph::getVersion ( void ) const
{
	return m_version;
}

// ======================================================================

#endif

