// ======================================================================
//
// Resolve3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// This is where all the different overlap/intersect/temporal/etc. tests
// come together and allow us to take two primitives and a motion step
// and produce their position at the end of the step.
//
// ======================================================================

#ifndef INCLUDED_Resolve3d_H
#define INCLUDED_Resolve3d_H

#include "sharedCollision/CollisionEnums.h"

class OrientedCylinder;
class YawedBox;
class Circle;
class Vector;
class Triangle3d;
class Segment3d;
class Plane3d;
class Ribbon3d;
class Line3d;
class Ray3d;
class Range;
class AxialBox;
class Sphere;
class OrientedBox;
class Ring;
class Cylinder;
class MultiShape;
class Contact;

namespace Resolve3d
{

// ----------------------------------------------------------------------

ResolutionResult ResolveContact_Slide3 ( Contact const & contact, Vector const & V, Vector & outV );

// ----------------------------------------------------------------------
// 2d resolution (need a Resolve2d)

ResolutionResult    ResolveCirclePoint      ( Circle const & A, Vector const & V, Vector   const & B, Vector & outV );
ResolutionResult    ResolveCircleCircle     ( Circle const & A, Vector const & V, Circle   const & B, Vector & outV );

ResolutionResult    ResolveCircleABox       ( Circle const & A, Vector const & V, AxialBox const & B, Vector & outV );
ResolutionResult    ResolveCircleYBox       ( Circle const & A, Vector const & V, YawedBox const & B, Vector & outV );

inline ResolutionResult Resolve ( Circle const & A, Vector const & V, Circle const & B, Vector & outV )   { return ResolveCircleCircle(A,V,B,outV); }
inline ResolutionResult Resolve ( Circle const & A, Vector const & V, AxialBox const & B, Vector & outV ) { return ResolveCircleABox(A,V,B,outV); }
inline ResolutionResult Resolve ( Circle const & A, Vector const & V, YawedBox const & B, Vector & outV ) { return ResolveCircleYBox(A,V,B,outV); }

// ----------

ResolutionResult    ResolveSphereSphere     ( Sphere const & A, Vector const & V, Sphere const & B, Vector & outV );
ResolutionResult    ResolveSphereCylinder   ( Sphere const & A, Vector const & V, Cylinder const & B, Vector & outV );
ResolutionResult    ResolveSphereOCylinder  ( Sphere const & A, Vector const & V, OrientedCylinder const & B, Vector & outV );
ResolutionResult    ResolveSphereABox       ( Sphere const & A, Vector const & V, AxialBox const & B, Vector & outV );
ResolutionResult    ResolveSphereYBox       ( Sphere const & A, Vector const & V, YawedBox const & B, Vector & outV );
ResolutionResult    ResolveSphereOBox       ( Sphere const & A, Vector const & V, OrientedBox const & B, Vector & outV );

ResolutionResult    ResolveCylinderSphere   ( Cylinder const & A, Vector const & V, Sphere   const & B, Vector & outV );
ResolutionResult    ResolveCylinderCylinder ( Cylinder const & A, Vector const & V, Cylinder const & B, Vector & outV );
ResolutionResult    ResolveCylinderABox     ( Cylinder const & A, Vector const & V, AxialBox const & B, Vector & outV );
ResolutionResult    ResolveCylinderYBox     ( Cylinder const & A, Vector const & V, YawedBox const & B, Vector & outV );


ResolutionResult    ResolveMultiShapes      ( MultiShape const & A, Vector const & V, MultiShape const & B, Vector & outV );

// ----------------------------------------------------------------------

inline ResolutionResult Resolve ( Sphere const & A, Vector const & V, Sphere const & B, Vector & outV )           { return ResolveSphereSphere(A,V,B,outV); }
inline ResolutionResult Resolve ( Sphere const & A, Vector const & V, Cylinder const & B, Vector & outV )         { return ResolveSphereCylinder(A,V,B,outV); }
inline ResolutionResult Resolve ( Sphere const & A, Vector const & V, OrientedCylinder const & B, Vector & outV ) { return ResolveSphereOCylinder(A,V,B,outV); }
inline ResolutionResult Resolve ( Sphere const & A, Vector const & V, AxialBox const & B, Vector & outV )         { return ResolveSphereABox(A,V,B,outV); }
inline ResolutionResult Resolve ( Sphere const & A, Vector const & V, YawedBox const & B, Vector & outV )         { return ResolveSphereYBox(A,V,B,outV); }
inline ResolutionResult Resolve ( Sphere const & A, Vector const & V, OrientedBox const & B, Vector & outV )      { return ResolveSphereOBox(A,V,B,outV); }

inline ResolutionResult Resolve ( Cylinder const & A, Vector const & V, Cylinder const & B, Vector & outV ) { return ResolveCylinderCylinder(A,V,B,outV); }
inline ResolutionResult Resolve ( Cylinder const & A, Vector const & V, AxialBox const & B, Vector & outV ) { return ResolveCylinderABox(A,V,B,outV); }
inline ResolutionResult Resolve ( Cylinder const & A, Vector const & V, YawedBox const & B, Vector & outV ) { return ResolveCylinderYBox(A,V,B,outV); }
inline ResolutionResult Resolve ( Cylinder const & A, Vector const & V, Sphere   const & B, Vector & outV ) { return ResolveCylinderSphere(A,V,B,outV); }



inline ResolutionResult Resolve ( MultiShape const & A, Vector const & V, MultiShape const & B, Vector & outV ) { return ResolveMultiShapes(A,V,B,outV); }

// ----------------------------------------------------------------------

}	// namespace Resolve3d

#endif // #ifndef INCLUDED_Resolve3d_H

