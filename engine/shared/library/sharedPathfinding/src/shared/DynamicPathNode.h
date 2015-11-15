// ======================================================================
//
// DynamicPathNode.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_DynamicPathNode_H
#define	INCLUDED_DynamicPathNode_H

#include "sharedMath/Vector.h"
#include "sharedPathfinding/PathNode.h"

#include <vector>

class PathEdge;
class DynamicPathGraph;

// ======================================================================

class DynamicPathNode : public PathNode
{
public:

	DynamicPathNode ( void );

	DynamicPathNode ( Vector const & position_p );

	virtual ~DynamicPathNode();

	// ----------

	int              getEdgeCount ( void ) const;
	PathEdge *       getEdge      ( int whichEdge );
	PathEdge const * getEdge      ( int whichEdge ) const;

	int              getNeighbor  ( int whichEdge ) const;

	bool             hasEdge      ( int nodeIndex ) const;
	bool             addEdge      ( int nodeIndex );
	bool             removeEdge   ( int nodeIndex );
	void             clearEdges   ( void );

protected:

	friend class DynamicPathGraph;

	virtual void  clearEdgeMarks     ( void ) const;
 	virtual int   markRedundantEdges ( void ) const;
	virtual int   removeMarkedEdges  ( void );

	bool  _removeEdge        ( int nodeIndex );

	DynamicPathGraph * _getGraph ( void );

	// ----------

	typedef std::vector<PathEdge> EdgeList;

	EdgeList m_edges;
};

// ======================================================================

#endif

