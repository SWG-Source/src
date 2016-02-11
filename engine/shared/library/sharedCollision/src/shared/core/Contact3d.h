// ======================================================================
//
// Contact3d.h
// copyright (c) 2001 Sony Online Entertainment
//
// Given two primitives and a motion step, compute their initial point
// of contact, the time of contact, and the contact normal
//
// ======================================================================

#ifndef INCLUDED_Contact3d_H
#define INCLUDED_Contact3d_H

#include "sharedMath/Vector.h"

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
class OrientedCylinder;
class Circle;
class YawedBox;
class MultiShape;
class BaseExtent;
class CollisionSurface;
class CellProperty;

// ----------

class Contact
{
public:

	Contact( bool newExists, 
		     bool newResolvable, 
			 float newTime, 
			 Vector newPoint, 
			 Vector newNormal )
	: m_exists(newExists),
	  m_resolvable(newResolvable),
	  m_time(newTime),
	  m_point(newPoint),
	  m_normal(newNormal),
	  m_surfaceId1(-1),
	  m_surfaceId2(-1),
	  m_cell(nullptr),
	  m_extent(nullptr),
	  m_surface(nullptr)
	{
	}

	Contact()
	{
	}

	static Contact noContact;
	static Contact cantResolve;

	// ----------

	bool	m_exists;
	bool    m_resolvable;

	float	m_time;
	Vector	m_point;
	Vector	m_normal;

	int     m_surfaceId1;
	int     m_surfaceId2;

	CellProperty const *     m_cell;
	BaseExtent const *       m_extent;
	CollisionSurface const * m_surface;
};

// ----------

namespace Contact3d
{

// ----------------------------------------------------------------------

Contact     ContactCirclePoint  ( Circle const & A, Vector const & V, Vector const & B );
Contact     ContactCircleCircle ( Circle const & A, Vector const & V, Circle const & B );
Contact     ContactCircleABox   ( Circle const & A, Vector const & V, AxialBox const & B );
Contact     ContactCircleYBox   ( Circle const & A, Vector const & V, YawedBox const & B );

// ----------

Contact     ContactSphereSphere     ( Sphere const & A, Vector const & V, Sphere const & B );
Contact     ContactSphereCylinder   ( Sphere const & A, Vector const & V, Cylinder const & B );
Contact     ContactSphereOCylinder  ( Sphere const & A, Vector const & V, OrientedCylinder const & B );
Contact     ContactSphereABox       ( Sphere const & A, Vector const & V, AxialBox const & B );
Contact     ContactSphereYBox       ( Sphere const & A, Vector const & V, YawedBox const & B );
Contact     ContactSphereOBox       ( Sphere const & A, Vector const & V, OrientedBox const & B );

// ----------

Contact     ContactCylinderSphere   ( Cylinder const & A, Vector const & V, Sphere const & B );

// ----------

Contact     ContactMultiShapes      ( MultiShape const & A, Vector const & V, MultiShape const & B );

// ----------
// Generics

inline Contact FindContact ( Circle const & A, Vector const & V, Vector const & B )           { return ContactCirclePoint(A,V,B); }
inline Contact FindContact ( Circle const & A, Vector const & V, Circle const & B )           { return ContactCircleCircle(A,V,B); }
inline Contact FindContact ( Circle const & A, Vector const & V, AxialBox const & B )         { return ContactCircleABox(A,V,B); }
inline Contact FindContact ( Circle const & A, Vector const & V, YawedBox const & B )         { return ContactCircleYBox(A,V,B); }

inline Contact FindContact ( Sphere const & A, Vector const & V, Sphere const & B )           { return ContactSphereSphere(A,V,B); }
inline Contact FindContact ( Sphere const & A, Vector const & V, Cylinder const & B )         { return ContactSphereCylinder(A,V,B); }
inline Contact FindContact ( Sphere const & A, Vector const & V, OrientedCylinder const & B ) { return ContactSphereOCylinder(A,V,B); }
inline Contact FindContact ( Sphere const & A, Vector const & V, AxialBox const & B )         { return ContactSphereABox(A,V,B); }
inline Contact FindContact ( Sphere const & A, Vector const & V, YawedBox const & B )         { return ContactSphereYBox(A,V,B); }
inline Contact FindContact ( Sphere const & A, Vector const & V, OrientedBox const & B )      { return ContactSphereOBox(A,V,B); }

inline Contact FindContact ( Cylinder const & A, Vector const & V, Sphere const & B )         { return ContactCylinderSphere(A,V,B); }

inline Contact FindContact ( MultiShape const & A, Vector const & V, MultiShape const & B )   { return ContactMultiShapes(A,V,B); }

// ----------------------------------------------------------------------

} // namespace Contact3d

#endif // #ifndef INCLUDED_Contact3d_H

