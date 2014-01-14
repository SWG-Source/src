// ======================================================================
//
// Triangle3d.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Triangle3d.h"

#include "sharedMath/Line3d.h"
#include "sharedMath/Plane3d.h"
#include "sharedMath/Segment3d.h"

// ----------

Triangle3d::Triangle3d( Vector const & a, Vector const & b, Vector const & c )
: m_cornerA(a),
  m_cornerB(b),
  m_cornerC(c),
  m_normal()
{
    calcNormal();
}

Triangle3d::Triangle3d()
: m_cornerA(),
  m_cornerB(),
  m_cornerC(),
  m_normal()
{
}

Vector const & Triangle3d::getCorner ( int i ) const
{
    return (&m_cornerA)[i % 3];
}

void Triangle3d::setCorner ( int i, Vector const & newCorner )
{
    (&m_cornerA)[i % 3] = newCorner;

    calcNormal();
}

Vector const &   Triangle3d::getCornerA  ( void ) const
{
    return m_cornerA;
}

Vector const &   Triangle3d::getCornerB  ( void ) const
{
    return m_cornerB;
}

Vector const &   Triangle3d::getCornerC  ( void ) const
{
    return m_cornerC;
}

void         Triangle3d::setCornerA  ( Vector const & newCorner )
{
    m_cornerA = newCorner;

    calcNormal();
}

void         Triangle3d::setCornerB  ( Vector const & newCorner )
{
    m_cornerB = newCorner;

    calcNormal();
}

void         Triangle3d::setCornerC  ( Vector const & newCorner )
{
    m_cornerC = newCorner;

    calcNormal();
}


Vector Triangle3d::getNormal(void) const 
{   
    return m_normal;
}

real Triangle3d::getArea ( void ) const
{
    return ((getCornerB()-getCornerA()).cross(getCornerC()-getCornerA())).magnitude() * 0.5f;
}

bool Triangle3d::isDegenerate    ( void ) const
{
    real area = getArea();

    return area < 0.000001f;    // one square millimeter
}

Plane3d          Triangle3d::getPlane        ( void ) const 
{ 
    return Plane3d(getCornerA(),getNormal()); 
}

// ----------

Vector       Triangle3d::getEdgeDir  ( int whichEdge ) const
{
    return getCorner(whichEdge+1) - getCorner(whichEdge);
}

Vector       Triangle3d::getEdgeDir0 ( void ) const 
{ 
    return getCornerB() - getCornerA();
}

Vector       Triangle3d::getEdgeDir1 ( void ) const 
{ 
    return getCornerC() - getCornerB(); 
}

Vector       Triangle3d::getEdgeDir2 ( void ) const 
{ 
    return getCornerA() - getCornerC();
}

// ----------

Segment3d        Triangle3d::getEdgeSegment  ( int whichSegment ) const
{
    return Segment3d(getCorner(whichSegment),getCorner(whichSegment+1));
}

Segment3d        Triangle3d::getEdgeSegment0 ( void ) const 
{ 
    return Segment3d(getCornerA(),getCornerB()); 
}

Segment3d        Triangle3d::getEdgeSegment1 ( void ) const 
{ 
    return Segment3d(getCornerB(),getCornerC()); 
}

Segment3d        Triangle3d::getEdgeSegment2 ( void ) const 
{ 
    return Segment3d(getCornerC(),getCornerA()); 
}

// ----------

Line3d           Triangle3d::getEdgeLine     ( int whichEdge ) const
{
    return Line3d(getCorner(whichEdge),getEdgeDir(whichEdge));
}

Line3d           Triangle3d::getEdgeLine0    ( void ) const 
{ 
    return Line3d(getCornerA(),getEdgeDir0()); 
}

Line3d           Triangle3d::getEdgeLine1    ( void ) const 
{ 
    return Line3d(getCornerB(),getEdgeDir1()); 
}

Line3d           Triangle3d::getEdgeLine2    ( void ) const 
{ 
    return Line3d(getCornerC(),getEdgeDir2()); 
}

// ----------

void             Triangle3d::flip            ( void )
{
    Vector temp = m_cornerC;
    m_cornerC = m_cornerB;
    m_cornerB = temp;
}

// ----------

bool             Triangle3d::isFacing        ( Vector const & dir ) const
{
    return getNormal().dot(dir) > 0;
}

// ----------

// assumes ***CLOCKWISE*** vertex winding

void             Triangle3d::calcNormal      ( void )
{
    Vector temp = getEdgeDir0().cross( getEdgeDir1() ); 
    
    IGNORE_RETURN( temp.normalize() );

    m_normal = temp;
}


