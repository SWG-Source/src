// ======================================================================
//
// Containment3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// Containment tests, bounding volume enclosure functions
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Containment3d_H
#define INCLUDED_Containment3d_H

class Vector;
class Triangle3d;
class Plane3d;
class Segment3d;

class Sphere;
class Cylinder;
class OrientedCylinder;
class AxialBox;
class YawedBox;
class OrientedBox;

class Torus;
class MultiShape;

#include "sharedCollision/CollisionEnums.h"

typedef std::vector<Vector>  VectorVector;

// ----------
// These methods should always be of the form

// ContainmentResult TestShapeBoundary ( Shape const & S, Boundary const & B );
// ContainmentResult TestShapeBoundary ( Shape const & S, Boundary const & B, real epsilon );

namespace Containment3d
{

// ----------------------------------------------------------------------
// Static non-epsilon containment tests - These are faster but may give 
// inconsistent results when the test shape touches or lies on the boundary

// ----------
// Point-vs-planar - containment means the point lies "behind" the plane

ContainmentResult  TestPointPlane       ( Vector const & V, Plane3d const & P );
ContainmentResult  TestPointTri         ( Vector const & V, Triangle3d const & T );

// ----------
// Planar-vs-planar - containment means the object lies "behind" the plane
                                  
ContainmentResult  TestTriTri           ( Triangle3d const & A, Triangle3d const & B );

// ----------
// Point-vs-volume

ContainmentResult  TestPointSphere      ( Vector const & A, Sphere const & B );
ContainmentResult  TestPointCylinder    ( Vector const & A, Cylinder const & B );
ContainmentResult  TestPointABox        ( Vector const & A, AxialBox const & B );
ContainmentResult  TestPointYBox        ( Vector const & A, YawedBox const & B );
ContainmentResult  TestPointOBox        ( Vector const & A, OrientedBox const & B );
ContainmentResult  TestPointTorus       ( Vector const & A, Torus const & B );
ContainmentResult  TestPointShape       ( Vector const & A, MultiShape const & B );

// ----------
// Volume-vs-volume

ContainmentResult  TestSphereSphere     ( Sphere const & A, Sphere const & B );
ContainmentResult  TestSphereCylinder   ( Sphere const & A, Cylinder const & B );
ContainmentResult  TestSphereOCylinder  ( Sphere const & A, OrientedCylinder const & B );
ContainmentResult  TestSphereABox       ( Sphere const & A, AxialBox const & B );
ContainmentResult  TestSphereYBox       ( Sphere const & A, YawedBox const & B );
ContainmentResult  TestSphereOBox       ( Sphere const & A, OrientedBox const & B );



ContainmentResult  TestCylinderSphere   ( Cylinder const & A, Sphere const & B );
ContainmentResult  TestCylinderCylinder ( Cylinder const & A, Cylinder const & B );
ContainmentResult  TestCylinderABox     ( Cylinder const & A, AxialBox const & B);
ContainmentResult  TestCylinderYBox     ( Cylinder const & A, YawedBox const & B );

ContainmentResult  TestABoxABox         ( AxialBox const & A, AxialBox const & B );

ContainmentResult  TestMultiShapes      ( MultiShape const & A, MultiShape const & B );

// ----------
// Aliases for convenience

inline ContainmentResult Test ( Vector     const & A, Plane3d    const & B ) { return TestPointPlane(A,B); }
inline ContainmentResult Test ( Vector     const & A, Triangle3d const & B ) { return TestPointTri(A,B); }

inline ContainmentResult Test ( Triangle3d const & A, Triangle3d const & B ) { return TestTriTri(A,B); }

// ----------

inline ContainmentResult Test ( Vector     const & A, Sphere      const & B ) { return TestPointSphere(A,B); }
inline ContainmentResult Test ( Vector     const & A, Cylinder    const & B ) { return TestPointCylinder(A,B); }
inline ContainmentResult Test ( Vector     const & A, AxialBox    const & B ) { return TestPointABox(A,B); }
inline ContainmentResult Test ( Vector     const & A, YawedBox    const & B ) { return TestPointYBox(A,B); }
inline ContainmentResult Test ( Vector     const & A, OrientedBox const & B ) { return TestPointOBox(A,B); }
inline ContainmentResult Test ( Vector     const & A, MultiShape  const & B ) { return TestPointShape(A,B); }

// ----------

inline ContainmentResult Test ( Sphere     const & A, Sphere     const & B ) { return TestSphereSphere(A,B); }
inline ContainmentResult Test ( Sphere     const & A, Cylinder   const & B ) { return TestSphereCylinder(A,B); }
inline ContainmentResult Test ( Sphere     const & A, OrientedCylinder const & B ) { return TestSphereOCylinder(A,B); }
inline ContainmentResult Test ( Sphere     const & A, AxialBox   const & B ) { return TestSphereABox(A,B); }
inline ContainmentResult Test ( Sphere     const & A, YawedBox   const & B ) { return TestSphereYBox(A,B); }
inline ContainmentResult Test ( Sphere     const & A, OrientedBox const & B ) { return TestSphereOBox(A,B); }

// ----------

inline ContainmentResult Test ( Cylinder   const & A, Sphere     const & B ) { return TestCylinderSphere(A,B); }
inline ContainmentResult Test ( Cylinder   const & A, Cylinder   const & B ) { return TestCylinderCylinder(A,B); }
inline ContainmentResult Test ( Cylinder   const & A, AxialBox   const & B ) { return TestCylinderABox(A,B); }
inline ContainmentResult Test ( Cylinder   const & A, YawedBox   const & B ) { return TestCylinderYBox(A,B); }

// ----------

inline ContainmentResult Test ( AxialBox   const & A, AxialBox   const & B ) { return TestABoxABox(A,B); }

// ----------

inline ContainmentResult Test ( MultiShape const & A, MultiShape const & B ) { return TestMultiShapes(A,B); }

// ----------
// these we're fudging a bit by running the test in the opposite order
// (which isn't technically correct)

inline ContainmentResult Test ( AxialBox         const & A, Cylinder   const & B )  { return Test(B,A); }
inline ContainmentResult Test ( YawedBox         const & A, Cylinder   const & B )  { return Test(B,A); }

inline ContainmentResult Test ( AxialBox         const & A, Sphere     const & B )  { return Test(B,A); }
inline ContainmentResult Test ( YawedBox         const & A, Sphere     const & B )  { return Test(B,A); }
inline ContainmentResult Test ( OrientedBox      const & A, Sphere     const & B )  { return Test(B,A); }
inline ContainmentResult Test ( OrientedCylinder const & A, Sphere     const & B )  { return Test(B,A); }


// ----------------------------------------------------------------------
// Epsilon tests - These require more computation but handle boundary 
// conditions correctly. Since we can't make assumptions about what
// a "good" epsilon would be, users must pass in their epsilon.

ContainmentResult  TestPointPlane   ( Vector const & V,
                                      Plane3d const & P,
                                      real epsilon );

ContainmentResult  TestPointTri     ( Vector const & V,
                                      Triangle3d const & T,
                                      real epsilon );

ContainmentResult  TestTriPlane     ( Triangle3d const & T,
                                      Plane3d const & P,
                                      real epsilon );

ContainmentResult  TestTriTri       ( Triangle3d const & A,
                                      Triangle3d const & B,
                                      real epsilon );

// ----------------------------------------------------------------------
// Enclosure functions

// These should be of the form

// Bounds EncloseBounds ( Primitive const & A, Primitive const & B, ... )

AxialBox           EncloseABox      ( VectorVector const & points );
AxialBox           EncloseABox      ( Sphere const & S );
AxialBox           EncloseABox      ( Segment3d const & S );
AxialBox           EncloseABox      ( Cylinder const & S );

AxialBox           EncloseABox      ( AxialBox const & A, AxialBox const & B );

Sphere             EncloseSphere    ( Sphere const & shape );
Sphere             EncloseSphere    ( Cylinder const & shape );
Sphere             EncloseSphere    ( OrientedCylinder const & shape );
Sphere             EncloseSphere    ( AxialBox const & shape );
Sphere             EncloseSphere    ( YawedBox const & shape );
Sphere             EncloseSphere    ( OrientedBox const & shape );

Sphere             EncloseSphere    ( Vector const & A, Vector const & B );
Sphere             EncloseSphere    ( Sphere const & S, Vector const & V );
Sphere             EncloseSphere    ( Sphere const & A, Sphere const & B );
Sphere             EncloseSphere    ( VectorVector const & points );

// ----------------------------------------------------------------------
// Clip functions - shrink/clip A to fit inside B, or return false if we can't

// These should be of the form

// bool ClipBounds ( Clippee const & A, Clipper const & B, Clippee & out );

bool               ClipSphere       ( Sphere const & A, Sphere const & B, Sphere & out );

// ----------------------------------------------------------------------

}; // namespace Containment3d

#endif
