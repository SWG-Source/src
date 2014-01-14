// ======================================================================
//
// PathNode.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_PathNode_H
#define	INCLUDED_PathNode_H

#include "sharedMath/Vector.h"
#include "sharedPathfinding/PathfindingEnums.h"

class Iff;
class PathGraph;

// ======================================================================

class PathNode
{
public:

	// ----------

	PathNode ( void );

	PathNode ( Vector const & position_p );
	
	virtual ~PathNode();

	virtual void      copy          ( PathNode const & P );

	// ----------

	PathGraph *       getGraph      ( void );
	PathGraph const * getGraph      ( void ) const;
	void              setGraph      ( PathGraph * graph );

	bool              hasSubgraph   ( void ) const;

	int               getIndex      ( void ) const;
	void              setIndex      ( int newIndex );

	int               getId         ( void ) const;
	void              setId         ( int newId );

	int               getKey        ( void ) const;
	void              setKey        ( int newKey );

	PathNodeType      getType       ( void ) const;
	void              setType       ( PathNodeType newType );

	int               getPartId     ( void ) const;
	void              setPartId     ( int newPart );

	// ----------

	Vector const &    getPosition_p ( void ) const;
	virtual void      setPosition_p ( Vector const & newPosition );

	float             getRadius     ( void ) const;
	void              setRadius     ( float newRadius );

	// ----------

	int               getUserId     ( void ) const;
	void              setUserId     ( int newId ) const;

	int               getMark       ( int whichMark ) const;
	void              setMark       ( int whichMark, int newValue ) const;
	void              clearMarks    ( void ) const;

	// ----------

	void              read_0000     ( Iff & iff );

	void              write         ( Iff & iff ) const;

	bool              isConcrete    ( void ) const;

protected:

	PathGraph *  m_graph;

	int          m_index;
	int          m_id;
	int          m_key;
	PathNodeType m_type;
	int          m_partId;

	Vector       m_position;
	float        m_radius;

	// These are used to speed up some algorithms. They're not persisted.
	// Code that uses the marks MUST clear them after use.

	mutable int  m_userId;
	mutable int  m_marks[4];
};

// ----------------------------------------------------------------------

inline PathGraph * PathNode::getGraph ( void )
{
	return m_graph;
}

inline PathGraph const * PathNode::getGraph ( void ) const
{
	return m_graph;
}

inline void PathNode::setGraph ( PathGraph * graph )
{
	m_graph = graph;
}

// ----------

inline int PathNode::getIndex ( void ) const
{
	return m_index;
}

inline void PathNode::setIndex ( int newIndex )
{
	m_index = newIndex;
}

// ----------

inline int PathNode::getId ( void ) const
{
	return m_id;
}

inline void PathNode::setId ( int newId )
{
	m_id = newId;
}

// ----------

inline int PathNode::getKey ( void ) const
{
	return m_key;
}

inline void PathNode::setKey ( int newKey )
{
	m_key = newKey;
}

// ----------

inline PathNodeType PathNode::getType ( void ) const
{
	return m_type;
}

inline void PathNode::setType ( PathNodeType newType )
{
	m_type = newType;
}

// ----------

inline int PathNode::getPartId ( void ) const
{
	return m_partId;
}

inline void PathNode::setPartId ( int newId )
{
	m_partId = newId;
}

// ----------------------------------------------------------------------

inline Vector const & PathNode::getPosition_p ( void ) const
{
	return m_position;
}

inline void PathNode::setPosition_p ( Vector const & newPosition )
{
	m_position = newPosition;
}

// ----------

inline float PathNode::getRadius ( void ) const
{
	return m_radius;
}

inline void PathNode::setRadius ( float newRadius )
{
	m_radius = newRadius;
}

// ----------------------------------------------------------------------

inline int PathNode::getUserId ( void ) const
{
	return m_userId;
}

inline void PathNode::setUserId ( int newId ) const
{
	m_userId = newId;
}

// ----------

inline int PathNode::getMark ( int whichMark ) const
{
	return m_marks[whichMark];
}

inline void PathNode::setMark ( int whichMark, int newMark ) const
{
	m_marks[whichMark] = newMark;
}

inline void PathNode::clearMarks ( void ) const
{
	m_marks[0] = -1;
	m_marks[1] = -1;
	m_marks[2] = -1;
	m_marks[3] = -1;
}

// ======================================================================

#endif

