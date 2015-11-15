// ======================================================================
//
// Intersect3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_Intersect3d_H
#define INCLUDED_Intersect3d_H

#include "sharedMath/Range.h"	// Need this for the inlined aliases to work
#include "sharedMath/Vector.h"

class Triangle3d;
class Segment3d;
class Plane3d;
class Ribbon3d;
class Line3d;
class Ray3d;
class Range;
class AxialBox;
class YawedBox;
class OrientedBox;
class OrientedCylinder;
class Sphere;
class Ring;
class Cylinder;
class Torus;
class Circle;
class MultiShape;

namespace Intersect3d
{

// ----------------------------------------------------------------------
// Intersection point calculation

bool    IntersectSegPlaneSided      ( Segment3d const & S, Plane3d const & P,    Vector & outPoint, float & outParam );
bool    IntersectSegPlaneUnsided    ( Segment3d const & S, Plane3d const & P,    Vector & outPoint, float & outParam );

bool    IntersectLinePlaneSided     ( Line3d const & L,    Plane3d const & P,    Vector & outPoint, float & outParam );
bool    IntersectLinePlaneUnsided   ( Line3d const & L,    Plane3d const & P,    Vector & outPoint, float & outParam );

bool    IntersectLineTriSided       ( Line3d const & L,    Triangle3d const & T, Vector & outPoint, float & outParam );
bool    IntersectLineTriUnsided     ( Line3d const & L,    Triangle3d const & T, Vector & outPoint, float & outParam );

bool    IntersectRayTriSided        ( Ray3d const & R,     Triangle3d const & T, Vector & outPoint, float & outParam );
bool    IntersectRayTriUnsided      ( Ray3d const & R,     Triangle3d const & T, Vector & outPoint, float & outParam );

bool    IntersectSegTriSided        ( Segment3d const & S, Triangle3d const & T, Vector & outPoint, float & outParam );
bool    IntersectSegTriUnsided      ( Segment3d const & S, Triangle3d const & T, Vector & outPoint, float & outParam );

bool    IntersectSegRibbonSided     ( Segment3d const & S,
                                      Ribbon3d const & R,
                                      float & outT,         // Segment's intersection param
                                      float & outU,         // Ribbon's intersection param (along the ribbon axis)
                                      float & outV );       // Ribbon's intersection param (across the ribbon)

// ----------

inline bool IntersectSided   ( Line3d const & L,    Triangle3d const & T, Vector & outPoint, float & outParam ) { return IntersectLineTriSided(L,T,outPoint,outParam); }
inline bool IntersectSided   ( Ray3d const & R,     Triangle3d const & T, Vector & outPoint, float & outParam ) { return IntersectRayTriSided(R,T,outPoint,outParam); }
inline bool IntersectSided   ( Segment3d const & S, Triangle3d const & T, Vector & outPoint, float & outParam ) { return IntersectSegTriSided(S,T,outPoint,outParam); }

inline bool IntersectUnsided ( Line3d const & L,    Triangle3d const & T, Vector & outPoint, float & outParam ) { return IntersectLineTriUnsided(L,T,outPoint,outParam); }
inline bool IntersectUnsided ( Ray3d const & R,     Triangle3d const & T, Vector & outPoint, float & outParam ) { return IntersectRayTriUnsided(R,T,outPoint,outParam); }
inline bool IntersectUnsided ( Segment3d const & S, Triangle3d const & T, Vector & outPoint, float & outParam ) { return IntersectSegTriUnsided(S,T,outPoint,outParam); }

// ----------------------------------------------------------------------
// Parametric linear-vs-plane

Range   IntersectLinePlane      ( Line3d const & line,      Plane3d const & plane );
Range   IntersectSegPlane       ( Segment3d const & seg,    Plane3d const & plane );

// ----------
// Parametric linear-vs-volume

Range   IntersectLineSphere     ( Line3d const & line,      Sphere const & sphere );
Range   IntersectLineABox       ( Line3d const & line,      AxialBox const & box );
Range   IntersectLineCylinder   ( Line3d const & line,      Cylinder const & cylinder );
Range   IntersectLineTorus      ( Line3d const & line,      Torus const & torus );
Range   IntersectLineShape      ( Line3d const & line,      MultiShape const & shape );

Range   IntersectRaySphere      ( Ray3d const & ray,        Sphere const & sphere );
Range   IntersectRayABox        ( Ray3d const & ray,        AxialBox const & box );
Range   IntersectRayCylinder    ( Ray3d const & ray,        Cylinder const & cylinder );
Range   IntersectRayTorus       ( Ray3d const & ray,        Torus const & torus );
Range   IntersectRayShape       ( Ray3d const & ray,        MultiShape const & shape );

Range   IntersectSegSphere      ( Segment3d const & seg,    Sphere const & sphere );
Range   IntersectSegABox        ( Segment3d const & seg,    AxialBox const & box );
Range   IntersectSegCylinder    ( Segment3d const & seg,    Cylinder const & cylinder );
Range   IntersectSegTorus       ( Segment3d const & seg,    Torus const & torus );
Range   IntersectSegShape       ( Segment3d const & seg,    MultiShape const & shape );

// ----------

struct ResultData
{
	ResultData();
	explicit ResultData(bool const getSurfaceNormal);
	bool const m_getSurfaceNormal;
	Vector m_surfaceNormal;
	float m_length;
private:
	ResultData(ResultData const & copy);
	ResultData & operator = (ResultData const & copy);

};

bool intersectRaySphereWithData(Ray3d const & ray, Sphere const &, ResultData * const);
bool intersectRayABoxWithData(Ray3d const & ray, AxialBox const &, ResultData * const);
bool intersectRayOBoxWithData(Ray3d const & ray, OrientedBox const &, ResultData * const);
bool intersectRayCylinderWithData(Ray3d const & ray, Cylinder const &, ResultData * const);
bool intersectRayOCylinderWithData(Ray3d const & ray, OrientedCylinder const &, ResultData * const);
bool intersectRayShapeWithData(Ray3d const & ray, MultiShape const &, ResultData * const);
bool intersectTriangleBox(AxialBox const & box, Vector const & point1, Vector const & point2, Vector const & point3);

// ----------
// Generic aliases

inline Range    Intersect       ( Line3d const & A,      Sphere const & B )     { return IntersectLineSphere(A,B); }
inline Range    Intersect       ( Line3d const & A,      AxialBox const & B )      { return IntersectLineABox(A,B); }
inline Range    Intersect       ( Line3d const & A,      Cylinder const & B ) { return IntersectLineCylinder(A,B); }
inline Range    Intersect       ( Line3d const & A,      Torus const & B )       { return IntersectLineTorus(A,B); }
inline Range    Intersect       ( Line3d const & A,      MultiShape const & B )  { return IntersectLineShape(A,B); }

inline Range    Intersect       ( Ray3d const & A,       Sphere const & B )     { return IntersectRaySphere(A,B); }
inline Range    Intersect       ( Ray3d const & A,       AxialBox const & B )      { return IntersectRayABox(A,B); }
inline Range    Intersect       ( Ray3d const & A,       Cylinder const & B ) { return IntersectRayCylinder(A,B); }
inline Range    Intersect       ( Ray3d const & A,       Torus const & B )       { return IntersectRayTorus(A,B); }
inline Range    Intersect       ( Ray3d const & A,        MultiShape const & B )  { return IntersectRayShape(A,B); }

inline Range    Intersect       ( Segment3d const & A,   Sphere const & B )     { return IntersectSegSphere(A,B); }
inline Range    Intersect       ( Segment3d const & A,   Cylinder const & B ) { return IntersectSegCylinder(A,B); }
inline Range    Intersect       ( Segment3d const & A,   AxialBox const & B )      { return IntersectSegABox(A,B); }
inline Range    Intersect       ( Segment3d const & A,   Torus const & B )       { return IntersectSegTorus(A,B); }
inline Range    Intersect       ( Segment3d const & A,   MultiShape const & B )  { return IntersectSegShape(A,B); }

// ----------------------------------------------------------------------
// Motion tests - when does A, moving with velocity V, hit B?

Range   IntersectPointPlane     ( Vector const & A, Vector const & V, Plane3d const & B );
Range   IntersectPointABox      ( Vector const & A, Vector const & V, AxialBox const & B );
Range   IntersectPointSphere    ( Vector const & A, Vector const & V, Sphere const & B );

Range   IntersectLineLine       ( Line3d const & A, Vector const & V, Line3d const & B );
Range   IntersectLineRay        ( Line3d const & A, Vector const & V, Ray3d const & B );
Range   IntersectLineSeg        ( Line3d const & A, Vector const & V, Segment3d const & B );

Range   IntersectSegPlane       ( Segment3d const & A, Vector const & V, Plane3d const & B );
Range   IntersectSegTri         ( Segment3d const & A, Vector const & V,Triangle3d const & B );

Range   IntersectRayTri         ( Ray3d const & A, Vector const & V, Triangle3d const & B );

Range   IntersectSpherePoint    ( Sphere const & A, Vector const & V, Vector const & B );
Range   IntersectSphereLine     ( Sphere const & A, Vector const & V, Line3d const & B );
Range   IntersectSphereXLine    ( Sphere const & A, Vector const & V, Vector const & B ); // Axis-aligned lines passing through point B
Range   IntersectSphereYLine    ( Sphere const & A, Vector const & V, Vector const & B );
Range   IntersectSphereZLine    ( Sphere const & A, Vector const & V, Vector const & B );
Range   IntersectSphereSeg      ( Sphere const & A, Vector const & V, Line3d const & B );
Range   IntersectSpherePlane    ( Sphere const & A, Vector const & V, Plane3d const & B );
Range   IntersectSphereRing     ( Sphere const & A, Vector const & V, Ring const & B );
Range   IntersectSphereSphere   ( Sphere const & A, Vector const & V, Sphere const & B );
Range   IntersectSphereCylinder ( Sphere const & A, Vector const & V, Cylinder const & B );
Range   IntersectSphereOCylinder( Sphere const & A, Vector const & V, OrientedCylinder const & B );
Range   IntersectSphereABox     ( Sphere const & A, Vector const & V, AxialBox const & B );
Range   IntersectSphereYBox     ( Sphere const & A, Vector const & V, YawedBox const & B );
Range   IntersectSphereOBox     ( Sphere const & A, Vector const & V, OrientedBox const & B );

Range   IntersectABoxABox       ( AxialBox const & A, Vector const & V, AxialBox const & B );

Range   IntersectMultiShapes    ( MultiShape const & A, Vector const & V, MultiShape const & B );

Range   IntersectCircleSeg_Below ( Circle const & A, Vector const & V, Segment3d const & S );

// ----------------------------------------------------------------------

inline Range Intersect          ( Sphere const & A, Vector const & V, Sphere const & B )         { return IntersectSphereSphere(A,V,B); }
inline Range Intersect          ( Sphere const & A, Vector const & V, Cylinder const & B )       { return IntersectSphereCylinder(A,V,B); }
inline Range Intersect          ( Sphere const & A, Vector const & V, OrientedCylinder const & B){ return IntersectSphereOCylinder(A,V,B); }
inline Range Intersect          ( Sphere const & A, Vector const & V, AxialBox const & B )       { return IntersectSphereABox(A,V,B); }
inline Range Intersect          ( Sphere const & A, Vector const & V, YawedBox const & B )       { return IntersectSphereYBox(A,V,B); }
inline Range Intersect          ( Sphere const & A, Vector const & V, OrientedBox const & B )    { return IntersectSphereOBox(A,V,B); }

inline Range Intersect          ( MultiShape const & A, Vector const & V, MultiShape const & B ) { return IntersectMultiShapes(A,V,B); }

// ----------------------------------------------------------------------

}; // namespace Collision3d

#endif

