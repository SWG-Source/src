// ======================================================================
//
// Sphere.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Sphere_H
#define INCLUDED_Sphere_H

// ======================================================================

#include "sharedMath/Vector.h"

class Range;
class Circle;

// ======================================================================

/// Describes a sphere in 3d space.

class Sphere
{
public:

	static const Sphere zero;
	static const Sphere unit;

public:

	// default destructor, copy constructor, assignment operator are all okay

	Sphere();
	Sphere(const Vector &center, real radius);
	Sphere(real x, real y, real z, real radius);

	void  setCenter(real x, real y, real z);
	void  setCenter(const Vector &center);
	void  setRadius(real radius);
	void  set(const Vector &center, float radius);
	void  set(float x, float y, float z, float radius);

	const Vector &getCenter() const;
	const real    getRadius() const;

	Vector const & getAxisX ( void ) const;
	Vector const & getAxisY ( void ) const;
	Vector const & getAxisZ ( void ) const;

	float getExtentX ( void ) const;
	float getExtentY ( void ) const;
	float getExtentZ ( void ) const;

	Circle getCircle ( void ) const;

	bool contains(const Vector &point) const;
	bool contains(const Sphere &other) const;

	bool intersectsLine(const Vector &startPoint, const Vector &endPoint) const;
	bool intersectsLineSegment(const Vector &startPoint, const Vector &endPoint) const;
	bool intersectsRay(const Vector & startPoint, const Vector & normalizedDirection) const;
	bool intersectsSphere(const Sphere &other) const;
	bool intersectsCone(Vector const & coneBase, Vector const & coneNormal, float coneAngleRadians) const;
	Vector closestPointOnSphere(Vector const & point) const;
	Vector approximateClosestPointOnSphere(Vector const & point) const;

 	bool operator==(const Sphere & rhs) const;

	Range getRangeX ( void ) const;
	Range getRangeY ( void ) const;
	Range getRangeZ ( void ) const;

private:

	/// Center point of the sphere.
	Vector m_center;

	/// Radius of the sphere.
	real   m_radius;
};

// ======================================================================
/**
 * Construct a default sphere.
 *
 * The sphere will be centered at (0,0,0) and have a radius of 0.
 */

inline Sphere::Sphere()
:
	m_center(),
	m_radius(0)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a sphere.
 *
 * @param center The center point for the sphere.
 * @param radius The radius of the sphere.
 */

inline Sphere::Sphere(const Vector &center, real radius)
:
	m_center(center),
	m_radius(radius)
{
	WARNING_STRICT_FATAL(m_radius < 0.0f, ("Sphere has negative radius!"));
	if(m_radius < 0.0f)
		radius = 0.0f;
}

// ----------------------------------------------------------------------
/**
 * Construct a sphere
 *
 * @param x      The X center point of the sphere.
 * @param y      The Y center point of the sphere.
 * @param z      The Z center point of the sphere.
 * @param radius The radius of the sphere.
 */

inline Sphere::Sphere(real x, real y, real z, real radius)
:
	m_center(Vector(x, y, z)),
	m_radius(radius)
{
	WARNING_STRICT_FATAL(m_radius < 0.0f, ("Sphere has negative radius!"));
	if(m_radius < 0.0f)
		radius = 0.0f;
}

// ----------------------------------------------------------------------
/**
 * Set the center point of the sphere.
 *
 * @param x The new X center point of the sphere.
 * @param y The new Y center point of the sphere.
 * @param z The new Z center point of the sphere.
 */

inline void Sphere::setCenter(const real x, const real y, const real z)
{
	m_center.x = x;
	m_center.y = y;
	m_center.z = z;
}

// ----------------------------------------------------------------------
/**
 * Set the center point of the sphere.
 *
 * @param center The new center point of the sphere.
 */

inline void Sphere::setCenter(const Vector &center)
{
	m_center = center;
}

// ----------------------------------------------------------------------
/**
 * Set the radius of the sphere.
 *
 * @param radius The new radius of the sphere.
 */

inline void Sphere::setRadius(real radius)
{
	m_radius = radius;
}

// ----------------------------------------------------------------------

inline void Sphere::set(const Vector &center, const float radius)
{
	m_center = center;
	m_radius = radius;
}

// ----------------------------------------------------------------------

inline void Sphere::set(const float x, const float y, const float z, const float radius)
{
	m_center.x = x;
	m_center.y = y;
	m_center.z = z;
	m_radius = radius;
}

// ----------------------------------------------------------------------
/**
 * Get the center point of the sphere.
 *
 * @return The center point of the sphere.
 */

inline const Vector &Sphere::getCenter() const
{
	return m_center;
}

// ----------------------------------------------------------------------
/**
 * Get the radius of the sphere.
 *
 * @return The radius of the sphere.
 */

inline const real    Sphere::getRadius() const
{
	return m_radius;
}

// ----------------------------------------------------------------------

inline Vector const & Sphere::getAxisX ( void ) const
{
	return Vector::unitX;
}

inline Vector const & Sphere::getAxisY ( void ) const
{
	return Vector::unitY;
}

inline Vector const & Sphere::getAxisZ ( void ) const
{
	return Vector::unitZ;
}

// ----------

inline float Sphere::getExtentX ( void ) const
{
	return getRadius();
}

inline float Sphere::getExtentY ( void ) const
{
	return getRadius();
}

inline float Sphere::getExtentZ ( void ) const
{
	return getRadius();
}

// ----------------------------------------------------------------------
/**
 * Check if a point is inside the sphere
 *
 * @return True if the point is in the sphere, otherwise false.
 */

inline bool Sphere::contains(const Vector &point) const
{
	return m_center.magnitudeBetweenSquared(point) <= sqr(m_radius);
}

// ----------------------------------------------------------------------
/**
 * Check if the sphere entirely contains another sphere.
 *
 * @return True if the second sphere is entirely contained within the first.
 */

inline bool Sphere::contains(const Sphere &other) const
{
	if (other.m_radius <= m_radius)
	{
		return m_center.magnitudeBetweenSquared(other.m_center) <= sqr(m_radius - other.m_radius);
	}
	else
		return false;
}

//-----------------------------------------------------------------------

inline bool Sphere::intersectsLine(const Vector & line0, const Vector & line1) const
{
	return contains(m_center.findClosestPointOnLine(line0, line1));
}

//-----------------------------------------------------------------------

inline bool Sphere::intersectsLineSegment(const Vector & startPoint, const Vector & endPoint) const
{
	return contains(m_center.findClosestPointOnLineSegment(startPoint, endPoint));
}

//-----------------------------------------------------------------------
/**
	@brief determine if a ray intersects a sphere
*/
inline bool Sphere::intersectsRay(const Vector & startPoint, const Vector & normalizedDirection) const
{
	// If the ray starts inside the sphere,
	// it MUST intersect the sphere
	if(contains(startPoint))
		return true;

	// get the squre magnitude between the origin of the sphere
	// and the start point of the ray
	Vector rayStartPointToCenter = m_center - startPoint;

	// project the ling segment described by the ray start point to the 
	// sphere origin on the ray
	const float originRayDot = rayStartPointToCenter.dot(normalizedDirection);

	if(originRayDot < 0.0f)
	{
		// Center of the sphere is 'behind' the ray, the sphere can't intersect the ray
		// without containing the start point, but since it doesn't the sphere doesn't intersect
		// the ray.

		return false;
	}

	// distanceSquared - sqr(originRayDot) = squre length of the line 
	// segment from the sphere origin that is perpendicular to the ray. 
	// If that length is less than the radius, there is an intersection.
	const float distanceSquared = rayStartPointToCenter.magnitudeSquared();
	const float distanceToRaySquared = sqr(m_radius) - (distanceSquared - sqr(originRayDot));
	return (distanceToRaySquared > 0.0f);
}

//-----------------------------------------------------------------------

inline bool Sphere::intersectsSphere(const Sphere & other) const
{
	const real d = m_center.magnitudeBetweenSquared(other.getCenter());
	const real r = sqr(m_radius + other.getRadius());
	return d < r;
}

//----------------------------------------------------------------------

inline bool Sphere::operator==(const Sphere & rhs) const
{
	return this == &rhs || (m_radius == rhs.m_radius && m_center == rhs.m_center);
}

// ======================================================================

#endif
