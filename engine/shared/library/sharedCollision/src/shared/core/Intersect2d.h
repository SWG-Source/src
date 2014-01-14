// ======================================================================
//
// Intersect2d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Intersect2d_H
#define INCLUDED_Intersect2d_H

class Segment3d;
class Circle;
class Vector;
class Range;
class Vector2d;
class Plane3d;
class AxialBox;
class YawedBox;
class Line3d;
class Ray3d;

// Note - 2d intersection routines performed with 3d primitives (Line3d, Segment3d, etc.)
// ignore the Y component of their inputs

namespace Intersect2d
{

// ----------------------------------------------------------------------

bool   IntersectLineLine ( Line3d const & A,
                           Line3d const & B,
                           float & outParamA,
                           float & outParamB );

Vector IntersectLineLine ( Line3d const & A,
                           Line3d const & B );

bool   IntersectRayRay   ( Ray3d const & A,
                           Ray3d const & B,
                           float & outParamA,
                           float & outParamB );

bool   IntersectRayRay   ( Ray3d const & A,
                           Ray3d const & B,
                           Vector & out );

bool   IntersectSegSeg   ( Segment3d const & A,
                           Segment3d const & B,
                           float & outParamA,
                           float & outParamB );

bool   IntersectLineSeg  ( Line3d const & A,
                           Segment3d const & B,
                           float & outParamA,
                           float & outParamB );

bool   IntersectRaySeg   ( Ray3d const & A,
                           Segment3d const & B,
                           float & outParamA,
                           float & outParamB );

// ----------

bool  IntersectSegCircle ( Segment3d const & S, 
                           Circle const & C, 
                           Vector & outPoint, 
                           float & outParam );

bool  IntersectSegSeg    ( Segment3d const & A,
                           Segment3d const & B,
                           Vector & outPoint,
                           float & outParamA,
                           float & outParamB );

// ----------------------------------------------------------------------

Range   IntersectPointSeg           ( Vector const & A, Vector const & V, Segment3d const & B );
Range   IntersectPointCircle        ( Vector const & A, Vector const & V, Circle const & B );
Range   IntersectPointABox          ( Vector const & A, Vector const & V, AxialBox const & B );

Range   IntersectCirclePoint        ( Circle const & A, Vector const & V, Vector const & B );
Range   IntersectCircleSeg          ( Circle const & A, Vector const & V, Segment3d const & B );
Range   IntersectCircleLine         ( Circle const & A, Vector const & V, Line3d const & B );
Range   IntersectCirclePlane        ( Circle const & A, Vector const & V, Plane3d const & B );
Range   IntersectCircleCircle       ( Circle const & A, Vector const & V, Circle const & B );
Range   IntersectCircleABox         ( Circle const & A, Vector const & V, AxialBox const & B );
Range   IntersectCircleYBox         ( Circle const & A, Vector const & V, YawedBox const & B );

// ----------
// Generic aliases

Range   Intersect              ( Vector const & A, Vector const & V, Circle const & B );
Range   Intersect              ( Circle const & A, Vector const & V, Circle const & B );
Range   Intersect              ( Circle const & A, Vector const & V, AxialBox const & B );
Range   Intersect              ( Circle const & A, Vector const & V, YawedBox const & B );

// ----------------------------------------------------------------------

} // namespace Intersect2d

#endif

