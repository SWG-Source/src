// ======================================================================
//
// Overlap3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Overlap3d_H
#define INCLUDED_Overlap3d_H

#include "sharedCollision/CollisionEnums.h"

class Triangle3d;
class Line3d;
class Segment3d;
class Ray3d;
class Vector;
class AxialBox;
class YawedBox;
class OrientedBox;
class Plane3d;
class Ribbon3d;
class Circle;
class Sphere;
class Range;
class Cylinder;
class MultiShape;

typedef std::vector<Vector> VertexList;

// ======================================================================

namespace Overlap3d
{

// ----------------------------------------------------------------------
// Linear-vs-planar overlap tests

HitResult     TestLineTri       ( Line3d const & L, Triangle3d const & T );
HitResult     TestRayTri        ( Ray3d const & R, Triangle3d const & T );
HitResult     TestSegTri        ( Segment3d const & S, Triangle3d const & T );
HitResult     TestSegPlane      ( Segment3d const & S, Plane3d const & P );
HitResult     TestSegRibbon     ( Segment3d const & S, Ribbon3d const & R );

// ----------------------------------------------------------------------
// Linear-vs-planar overlap tests

bool    TestLineTriSided        ( Line3d const & L, Triangle3d const & T );
bool    TestLineTriUnsided      ( Line3d const & L, Triangle3d const & T );

bool    TestRayTriSided         ( Ray3d const & R, Triangle3d const & T );
bool    TestRayTriUnsided       ( Ray3d const & R, Triangle3d const & T );

bool    TestSegTriSided         ( Segment3d const & S, Triangle3d const & T );
bool    TestSegTriUnsided       ( Segment3d const & S, Triangle3d const & T );

bool    TestSegPlaneSided       ( Segment3d const & S, Plane3d const & P );
bool    TestSegPlaneUnsided     ( Segment3d const & S, Plane3d const & P );

bool    TestSegRibbonSided      ( Segment3d const & S, Ribbon3d const & R );
bool    TestSegRibbonUnsided    ( Segment3d const & S, Ribbon3d const & R );

bool    TestSegCircle           ( Segment3d const & S, Circle const & C );

bool    TestSegCircle_Below     ( Segment3d const & S, Circle const & C ); // returns true if the circle overlaps the ribbon created by projecting the segment downwards

// ----------
// Linear-vs-volume overlap tests

bool    TestRayABox             ( Ray3d const & R, AxialBox const & box );
bool    TestLineABox            ( Line3d const & L, AxialBox const & box );
bool    TestSegABox             ( Segment3d const & S, AxialBox const & B );

bool    TestYLineCircle         ( Vector const & V, Circle const & C );
bool    TestYLineSphere         ( Vector const & V, Sphere const & S );
bool    TestYLineCylinder       ( Vector const & V, Cylinder const & C );
bool    TestYLineABox           ( Vector const & V, AxialBox const & B );	// Y-axis-aligned line passing through V
bool    TestYLineYBox           ( Vector const & V, YawedBox const & B );	
bool    TestYLineOBox           ( Vector const & V, OrientedBox const & B );	

bool    TestYLineShape          ( Vector const & V, MultiShape const & S );

inline bool TestYLine ( Vector const & V, Circle const & C )       { return TestYLineCircle(V,C); }
inline bool TestYLine ( Vector const & V, Sphere const & S )       { return TestYLineSphere(V,S); }
inline bool TestYLine ( Vector const & V, Cylinder const & C )     { return TestYLineCylinder(V,C); }
inline bool TestYLine ( Vector const & V, AxialBox const & B )     { return TestYLineABox(V,B); }
inline bool TestYLine ( Vector const & V, YawedBox const & B )     { return TestYLineYBox(V,B); }
inline bool TestYLine ( Vector const & V, OrientedBox const & B )  { return TestYLineOBox(V,B); }
inline bool TestYLine ( Vector const & V, MultiShape const & S )   { return TestYLineShape(V,S); }

// ----------
// Moving point-vs-volume tests

bool    TestPointABox           ( Vector const & P, Vector const & V, AxialBox const & B );

// ----------
// Planar-vs-planar overlap tests

bool    TestTriTri              ( Triangle3d const & A, Triangle3d const & B );

// ----------
// Volume-vs-planar overlap tests

bool    TestSpherePlane         ( Sphere const & A, Plane3d const & B );
bool    TestSpherePoly          ( Sphere const & A, VertexList const & B, Plane3d const & P );

// ----------
// Volume-vs-volume overlap tests

bool    TestABoxABox            ( AxialBox const & A, AxialBox const & B );
bool    TestOBoxOBox            ( OrientedBox const & A, OrientedBox const & B );

bool    TestSphereABox          ( Sphere const & A, AxialBox const & B );
bool    TestSphereYBox          ( Sphere const & A, YawedBox const & B );
bool    TestSphereOBox          ( Sphere const & A, OrientedBox const & B );

bool    TestYSlabABox           ( Range const & A, AxialBox const & B );
bool    TestYSlabYBox           ( Range const & A, AxialBox const & B );
bool    TestYSlabOBox           ( Range const & A, AxialBox const & B );


// ----------
// Projected overlap tests - determine if two primitives overlap when
// projected onto a plane perpendicular to the given view direction.

bool    TestTriTri_proj         ( Triangle3d const & triA,
                                  Triangle3d const & triB,
                                  Vector const & direction,
                                  real epsilon );

bool    TestSegSeg_proj         ( Segment3d const & segA,
                                  Segment3d const & segB,
                                  Vector const & direction );


// ----------------------------------------------------------------------
// Aliases for templates

inline bool TestSided   ( Line3d const & L,    Triangle3d const & T )  { return TestLineTriSided(L,T); }
inline bool TestSided   ( Ray3d const & R,     Triangle3d const & T )  { return TestRayTriSided(R,T); }
inline bool TestSided   ( Segment3d const & S, Triangle3d const & T )  { return TestSegTriSided(S,T); }

inline bool TestUnsided ( Line3d const & L,    Triangle3d const & T )  { return TestLineTriUnsided(L,T); }
inline bool TestUnsided ( Ray3d const & R,     Triangle3d const & T )  { return TestRayTriUnsided(R,T); }
inline bool TestUnsided ( Segment3d const & S, Triangle3d const & T )  { return TestSegTriUnsided(S,T); }

inline bool Test        ( Line3d const & L,    AxialBox const & B )    { return TestLineABox(L,B); }
inline bool Test        ( Ray3d const & R,     AxialBox const & B )    { return TestRayABox(R,B); }
inline bool Test        ( Segment3d const & S, AxialBox const & B )    { return TestSegABox(S,B); }

inline bool Test        ( AxialBox const & A,  AxialBox const & B )    { return TestABoxABox(A,B); }

// ----------------------------------------------------------------------

};  // namespace Collision3d

#endif
