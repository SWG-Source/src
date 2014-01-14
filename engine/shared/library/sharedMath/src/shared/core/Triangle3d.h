// ======================================================================
//
// Triangle3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Triangle3d_H
#define INCLUDED_Triangle3d_H

#include "sharedMath/Vector.h"

class Line3d;
class Plane3d;
class Segment3d;

// ======================================================================
// 3d Triangle3d

// ***CLOCKWISE*** vertex winding!

class Triangle3d
{
public:

	Triangle3d( Vector const & a, Vector const & b, Vector const & c );
	Triangle3d();

	// ----------

	Vector const &  getCorner       ( int i ) const;
	Vector const &  getCornerA      ( void ) const;
	Vector const &  getCornerB      ( void ) const;
	Vector const &  getCornerC      ( void ) const;

	void            setCorner       ( int i, Vector const & newCorner );
	void            setCornerA      ( Vector const & newCorner );
	void            setCornerB      ( Vector const & newCorner );
	void            setCornerC      ( Vector const & newCorner );

	Vector          getNormal       ( void ) const;

	// ----------

	real            getArea         ( void ) const;
	bool            isDegenerate    ( void ) const;

	bool            isFacing        ( Vector const & dir ) const;
	void            flip            ( void ); // Swaps vertices B and C, so that the triangle will face the other direction

	// ----------

	Plane3d         getPlane        ( void ) const;

	Vector          getEdgeDir      ( int whichEdge ) const;
	Vector          getEdgeDir0     ( void ) const;
	Vector          getEdgeDir1     ( void ) const;
	Vector          getEdgeDir2     ( void ) const;

	Segment3d       getEdgeSegment  ( int whichEdge ) const;
	Segment3d       getEdgeSegment0 ( void ) const;
	Segment3d       getEdgeSegment1 ( void ) const;
	Segment3d       getEdgeSegment2 ( void ) const;

	Line3d          getEdgeLine     ( int whichEdge ) const;
	Line3d          getEdgeLine0    ( void ) const;
	Line3d          getEdgeLine1    ( void ) const;
	Line3d          getEdgeLine2    ( void ) const;

	// ----------

protected:

	void            calcNormal      ( void );

	Vector m_cornerA;
	Vector m_cornerB;
	Vector m_cornerC;
	Vector m_normal;
};

// ----------------------------------------------------------------------

#endif

