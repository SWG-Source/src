// ======================================================================
//
// Containment2d.h
// copyright (c) 2001 Sony Online Entertainment
//
// Containment tests, bounding volume enclosure functions
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Containment2d_H
#define INCLUDED_Containment2d_H

class AxialBox;
class YawedBox;
class Sphere;
class Circle;
class Vector;
class Vector2d;
class Line3d;
class Segment3d;

#include "sharedCollision/CollisionEnums.h"

namespace Containment2d
{

// ----------------------------------------------------------------------
// Point-vs-line

// The current Line2d is not in point-normal form, so this takes a point and direction instead

ContainmentResult TestPointLine		( Vector2d const & P, Vector2d const & B, Vector2d const & D );

ContainmentResult TestPointLine		( Vector const & P, Line3d const & L );

ContainmentResult TestPointSeg      ( Vector const & P, Segment3d const & S );

ContainmentResult TestPointSeg      ( Vector const & P, Vector const & B, Vector const & E );
	
// ----------
// Point-vs-volume

ContainmentResult TestPointABox		( Vector const & V, AxialBox const & B );
ContainmentResult TestPointCircle   ( Vector const & V, Circle const & C );

// ----------
// Volume-vs-volume

ContainmentResult TestCircleABox	( Circle const & C, AxialBox const & B );
ContainmentResult TestCircleYBox	( Circle const & C, YawedBox const & B );
ContainmentResult TestCircleCircle	( Circle const & A, Circle const & B );

// ----------------------------------------------------------------------

}; // namespace Containment2d

#endif

