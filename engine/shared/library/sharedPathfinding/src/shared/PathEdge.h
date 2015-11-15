// ======================================================================
//
// PathEdge.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_PathEdge_H
#define	INCLUDED_PathEdge_H

class PathEdge;

class Iff;

// ======================================================================

class PathEdge
{
public:

	PathEdge ( void );

	PathEdge ( int indexA, int indexB );
	
	virtual ~PathEdge();

	// ----------

	bool  operator <  ( PathEdge const & edge ) const;
	bool  operator == ( PathEdge const & edge ) const;

	int   getIndexA   ( void ) const;
	int   getIndexB   ( void ) const;

	void  setIndexA   ( int newIndex );
	void  setIndexB   ( int newIndex );

	void  read_old    ( Iff & iff );
	void  read_0000   ( Iff & iff );

	void  write       ( Iff & iff ) const;

	// ----------

	int   getUserId   ( void ) const;
	void  setUserId   ( int newId ) const;

	int   getMark     ( int whichMark ) const;
	void  setMark     ( int whichMark, int newValue ) const;
	void  clearMarks  ( void ) const;

protected:

	int m_indexA;
	int m_indexB;

	float m_laneWidthRight;	// [1]
	float m_laneWidthLeft;

	// These are used to speed up some algorithms. They're not persisted.

	mutable int  m_userId;
	mutable int  m_marks[4];
};

// [1] - The lane widths indicate how far to the left or the right of 
// the edge a creature moving along the edge can stray without hitting 
// anything.

// ----------------------------------------------------------------------

inline bool PathEdge::operator <  ( PathEdge const & edge ) const
{
	if( m_indexA == edge.m_indexA )
	{
		return m_indexB < edge.m_indexB;
	}
	else
	{
		return m_indexA < edge.m_indexA;
	}
}

inline bool PathEdge::operator == ( PathEdge const & edge ) const
{
	return (m_indexA == edge.m_indexA) && (m_indexB == edge.m_indexB);
}

// ----------

inline int PathEdge::getIndexA ( void ) const
{
	return m_indexA;
}

inline int PathEdge::getIndexB ( void ) const
{
	return m_indexB;
}

// ----------

inline void PathEdge::setIndexA ( int newIndex )
{
	m_indexA = newIndex;
}

inline void PathEdge::setIndexB( int newIndex )
{
	m_indexB = newIndex;
}

// ----------

inline int PathEdge::getUserId ( void ) const
{
	return m_userId;
}

inline void PathEdge::setUserId ( int newId ) const
{
	m_userId = newId;
}

// ----------

inline int PathEdge::getMark ( int whichMark ) const
{
	return m_marks[whichMark];
}

inline void PathEdge::setMark ( int whichMark, int newMark ) const
{
	m_marks[whichMark] = newMark;
}

inline void PathEdge::clearMarks ( void ) const
{
	m_marks[0] = -1;
	m_marks[1] = -1;
	m_marks[2] = -1;
	m_marks[3] = -1;
}

// ======================================================================

#endif

