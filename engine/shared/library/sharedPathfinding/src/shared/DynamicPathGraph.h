// ======================================================================
//
// DynamicPathGraph.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_DynamicPathGraph_H
#define	INCLUDED_DynamicPathGraph_H

#include "sharedPathfinding/PathGraph.h"

#include "sharedMath/Vector.h"

class DynamicPathNode;

// ======================================================================
// Dynamic path graph, used for graphs where we need to be able to add
// and remove nodes on the fly (such as city graphs)

// DynamicPathGraph uses a sparse array to store its nodes. Calling
// getNode with a nodeIndex in [0,nodeCount) may return nullptr. If you
// want to know the number of live nodes in the graph, call getLiveNodeCount.

class DynamicPathGraph : public PathGraph
{
public:

	DynamicPathGraph ( PathGraphType type = PGT_None );

	virtual ~DynamicPathGraph();

	// ----------
	// PathGraph interface

	virtual int               getNodeCount    ( void ) const;
	virtual PathNode *        getNode         ( int nodeIndex );
	virtual PathNode const *  getNode         ( int nodeIndex ) const;

	virtual int               getEdgeCount    ( int nodeIndex ) const;
	virtual PathEdge *        getEdge         ( int nodeIndex, int edgeIndex );
	virtual PathEdge const *  getEdge         ( int nodeIndex, int edgeIndex ) const;

	// ----------

	virtual int               addNode         ( DynamicPathNode * newNode );
	virtual void              removeNode      ( int nodeIndex );

	virtual void              moveNode        ( int nodeIndex, Vector const & newPosition );

	virtual void              update          ( void );

	virtual bool              isDirty         ( void ) const;
	virtual void              clean           ( void );

	virtual void              clear           ( void );

	virtual void              unlinkNode      ( int nodeIndex );
	virtual void              relinkNode      ( int nodeIndex );

	virtual int               getLiveNodeCount( void ) const;

protected:

	friend class DynamicPathNode;

	DynamicPathNode *         _getNode        ( int nodeIndex );
	DynamicPathNode const *   _getNode        ( int nodeIndex ) const;

	typedef std::vector<DynamicPathNode *> NodeList;
	typedef std::vector<int> IndexList;

	NodeList *  m_nodeList;
	IndexList * m_dirtyNodes;
	int         m_liveNodeCount;
};

// ======================================================================

#endif

