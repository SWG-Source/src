// ======================================================================
//
// Distance2d.h
// copyright (c) 2001 Sony Online Entertainment
//
// Containment tests, bounding volume enclosure functions
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Distance2d_H
#define INCLUDED_Distance2d_H

#include "sharedCollision/CollisionEnums.h"

class Vector;
class AxialBox;
class Quadratic;
class YawedBox;
class Circle;
class Line3d;
class Ray3d;
class Range;
class Segment3d;
class Sphere;
class Cylinder;

// ----------------------------------------------------------------------

namespace Distance2d
{

// ----------
// Static 2d distance

float       DistancePointPoint      ( Vector const & A, Vector const & B );
float       DistancePointLine       ( Vector const & V, Line3d const & L );
float       DistancePointRay        ( Vector const & V, Ray3d const & R );
float       DistancePointSeg        ( Vector const & V, Segment3d const & S );

float       DistancePointSeg        ( Vector const & V, Vector const & begin, Vector const & end );

float       DistancePointSphere     ( Vector const & V, Sphere const & S );
float       DistancePointCylinder   ( Vector const & V, Cylinder const & C );

// Static 2d distance squared

float       Distance2PointPoint     ( Vector const & V, Vector const & B );
float       Distance2PointLine      ( Vector const & V, Line3d const & L );
float       Distance2PointRay       ( Vector const & V, Ray3d const & R );
float       Distance2PointSeg       ( Vector const & V, Segment3d const & S );

float       Distance2PointSeg       ( Vector const & V, Vector const & begin, Vector const & end );


inline float Distance               ( Vector const & A, Vector const & B ) { return DistancePointPoint(A,B); }

inline float Distance2              ( Vector const & A, Vector const & B ) { return Distance2PointPoint(A,B); }

// ----------
// Dynamic distance (returns distance squared as a function of time)

Quadratic   DistancePointPoint      ( Vector const & A, 
                                      Vector const & V, 
                                      Vector const & B );

// ----------

float       ClosestParamLine        ( Vector const & V, Line3d const & L );

Vector      ClosestPointLine        ( Vector const & V, Line3d const & L );
Vector      ClosestPointRay         ( Vector const & V, Ray3d const & R );
Vector      ClosestPointSeg         ( Vector const & V, Segment3d const & S );

Vector      ClosestPointCircle      ( Vector const & V, Circle const & C );
Vector      ClosestPointLine        ( Vector const & V, Line3d const & L );
Vector      ClosestPointABox        ( Vector const & V, AxialBox const & B );
Vector      ClosestPointYBox        ( Vector const & V, YawedBox const & B );

float       ProjectAxis             ( Line3d const & L, Vector const & V );
Range       ProjectAxis             ( Line3d const & L, Circle const & C );

// ----------

}; // namespace Distance2d

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_Distance2d_H

