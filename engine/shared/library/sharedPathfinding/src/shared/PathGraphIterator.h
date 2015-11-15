// ======================================================================
//
// PathGraphIterator.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_PathGraphIterator_H
#define	INCLUDED_PathGraphIterator_H

class PathGraph;
class PathNode;

// ======================================================================
// This is an accessory class to make code that traverses path graphs
// easier. It's not an iterator in the strict STL sense, but it is quite
// helpful.

class PathGraphIterator
{
public:

	PathGraphIterator ();

	PathGraphIterator ( PathGraph const * graph, int nodeIndex );

	// ----------

	bool             isValid          ( void ) const;

	PathNode const * getNode          ( void ) const;

	int              getNeighborCount ( void ) const;
	PathNode const * getNeighbor      ( int whichNeighbor ) const;

	void             stepTo           ( int whichNeighbor );

	// ----------

protected:

	PathGraph const * m_graph;
	int               m_nodeIndex;
};

// ======================================================================

#endif

