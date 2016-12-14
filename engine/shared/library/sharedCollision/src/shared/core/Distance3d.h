// ======================================================================
//
// Distance3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// Distance between two primitives, closest points, separating planes
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Distance3d_H
#define INCLUDED_Distance3d_H

class Vector;
class Triangle3d;
class Plane3d;
class Segment3d;
class Line3d;
class Cylinder;
class OrientedCylinder;
class AxialBox;
class YawedBox;
class OrientedBox;
class OrientedCircle;
class Quadratic;
class Range;
class Sphere;
class MultiShape;

typedef std::vector<Vector> VertexList;

#include "sharedCollision/CollisionEnums.h"
#include "sharedMath/Vector.h"

// ----------------------------------------------------------------------
// 3d operations

namespace Distance3d
{

// ----------------------------------------------------------------------
// Closest distance computation

float    DistancePointPoint     ( Vector const & A, Vector const & B );

float    DistancePointLine      ( Vector const & V, Line3d const & L );
float    DistancePointSeg       ( Vector const & V, Segment3d const & S );

float    DistancePointPlane     ( Vector const & V, Plane3d const & P );	// this is signed distance - negative is behind the plane
float    DistancePointTri       ( Vector const & V, Triangle3d const & T );
float    DistancePointPoly      ( Vector const & V, VertexList const & vertices );

float    DistancePointSphere    ( Vector const & V, Sphere const & S );
float    DistancePointCylinder  ( Vector const & V, Cylinder const & C );
float    DistancePointABox      ( Vector const & V, AxialBox const & B );
float    DistancePointYBox      ( Vector const & V, YawedBox const & B );
float    DistancePointOBox      ( Vector const & V, OrientedBox const & B );

float    DistancePointShape     ( Vector const & V, MultiShape const & S );

float    DistanceLineLine       ( Line3d const & A, Line3d const & B );

float    DistanceSphereCylinder ( Sphere const & S, Cylinder const & C );

// ----------

inline float DistanceBetween    ( Vector const & V, Sphere const & S )       { return DistancePointSphere(V,S); }
inline float DistanceBetween    ( Vector const & V, Cylinder const & C )     { return DistancePointCylinder(V,C); }
inline float DistanceBetween    ( Vector const & V, AxialBox const & B )     { return DistancePointABox(V,B); }
inline float DistanceBetween    ( Vector const & V, YawedBox const & B )     { return DistancePointYBox(V,B); }
inline float DistanceBetween    ( Vector const & V, OrientedBox const & B )  { return DistancePointOBox(V,B); }
inline float DistanceBetween    ( Vector const & V, MultiShape const & S )   { return DistancePointShape(V,S); }

inline float DistanceBetween    ( Sphere const & S, Cylinder const & C )     { return DistanceSphereCylinder(S,C); }

inline float DistanceBetween    ( Cylinder const & C, Sphere const & S )     { return DistanceSphereCylinder(S,C); }

// ----------------------------------------------------------------------
// Closest distance squared computation

float    Distance2PointPoint    ( Vector const & A, Vector const & B );

float    Distance2PointPoly     ( Vector const & V, VertexList const & vertices );
float    Distance2PointABox     ( Vector const & V, AxialBox const & B );

// ----------------------------------------------------------------------
// Farthest distance computation

float   FarDistancePointSphere  ( Vector const & V, Sphere const & S );

// ----------------------------------------------------------------------
// Closest point calculation

// These always return the point on (the boundary of) A that's closest to B.

Vector  ClosestPointLine        ( Vector const & V, Line3d const & L );
Vector  ClosestPointSeg         ( Vector const & V, Segment3d const & S );

Vector  ClosestPointPlane       ( Vector const & V, Plane3d const & P );

Vector  ClosestPointTri         ( Vector const & V, Triangle3d const & T );
Vector  ClosestPointPoly        ( Vector const & V, VertexList const & vertices );

// ----------

Vector  ClosestPointSphere      ( Vector const & V, Sphere const & S );
Vector  ClosestPointCylinder    ( Vector const & V, Cylinder const & C );
Vector  ClosestPointABox        ( Vector const & V, AxialBox const & B );
Vector  ClosestPointYBox        ( Vector const & V, YawedBox const & B );
Vector  ClosestPointOBox        ( Vector const & V, OrientedBox const & B );

// closest point on S to a y-axis-aligned line passing through V

Vector  ClosestPointYLine        ( Segment3d const & S, Vector const & V );

// ----------

inline Vector ClosestPoint ( Vector const & V, Sphere const & S )      { return ClosestPointSphere(V,S); }
inline Vector ClosestPoint ( Vector const & V, Cylinder const & S )    { return ClosestPointCylinder(V,S); }
inline Vector ClosestPoint ( Vector const & V, AxialBox const & S )    { return ClosestPointABox(V,S); }
inline Vector ClosestPoint ( Vector const & V, YawedBox const & S )    { return ClosestPointYBox(V,S); }
inline Vector ClosestPoint ( Vector const & V, OrientedBox const & S ) { return ClosestPointOBox(V,S); }

// ----------------------------------------------------------------------
// Farthest point calculation

Vector FarthestPoint            ( Vector const & V, Sphere const & S );
Vector FarthestPoint            ( Vector const & V, Cylinder const & S );
Vector FarthestPoint            ( Vector const & V, AxialBox const & S );
Vector FarthestPoint            ( Vector const & V, YawedBox const & S );
Vector FarthestPoint            ( Vector const & V, OrientedBox const & S );

// ----------------------------------------------------------------------
// Closest point calculation, parametric

bool    ClosestPairLineLine     ( Line3d const & A, 
                                  Line3d const & B, 
                                  float & outA, 
                                  float & outB );

// ----------------------------------------------------------------------
// Separating plane test (returns true if the plane separates the two objects)

bool    TestSeparate            ( Plane3d & P,
                                  Triangle3d const & A,
                                  Triangle3d const & B,
                                  float epsilon );

// ----------------------------------------------------------------------
// Separating plane calculation

bool    Separate                ( Triangle3d const & A,
                                  Triangle3d const & B,
                                  Plane3d & out,
                                  float epsilon );

// ----------------------------------------------------------------------
// Determine if two objects are separated by 

bool	TestSeparated   ( ContainmentResult A, ContainmentResult B );

bool    TestSeparated   ( Triangle3d const & A, Triangle3d const & B, Plane3d const & P, float epsilon );

bool    TestCoplanar    ( Triangle3d const & A, Triangle3d const & B, float epsilon );

// ----------------------------------------------------------------------
// Axial projections - project a primitive onto a line to produce a 1d 
// range in line-parametric space

float   ProjectAxis     ( Line3d const & L, Vector const & V );

Range   ProjectAxis     ( Line3d const & L, AxialBox const & B );
Range   ProjectAxis     ( Line3d const & L, OrientedBox const & B );
Range   ProjectAxis     ( Line3d const & L, Sphere const & S );
Range   ProjectAxis     ( Line3d const & L, OrientedCircle const & S );
Range   ProjectAxis     ( Line3d const & L, Segment3d const & C );

// ----------------------------------------------------------------------
// Moving-point-vs-point distance - returns squared distance 
// as a function of time.

Quadratic	DistancePointPoint	( Vector const & A, 
								  Vector const & V,
								  Vector const & B );

// ----------------------------------------------------------------------

};  // namespace Collision3d

// ----------------------------------------------------------------------

#endif
