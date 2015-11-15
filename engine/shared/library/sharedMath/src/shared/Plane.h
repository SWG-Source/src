// ======================================================================
//
// Plane.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef PLANE_H
#define PLANE_H

// ======================================================================

#include "sharedMath/Vector.h"

class Transform;

// ======================================================================

// Class to contain data about a plane.
//
// The plane is described as a normal {A,B,C} and a D plane coefficient
// making the following equation true: Ax + By + Cz + D = 0.

class Plane
{
private:

	// The plane's normal
	Vector   normal;

	// The plane's D coefficient
	real     d;

public:

	Plane(void);
	Plane(const Vector &newNormal, real newD);
	Plane(const Vector &point0, const Vector &point1, const Vector &point2);
	Plane(const Vector &normal, const Vector &pointOnPlane);

	void set(const Vector &newNormal, real newD);
	void set(const Vector &point0, const Vector &point1, const Vector &point2);
	void set(const Vector &newNormal, const Vector &pointOnPlane);
	void set(const Plane &other, const Transform &trans);

	void transform (const Transform &trans);
	void transform_p2l(const Transform &trans);

	const Vector &getNormal(void) const;
	const real    getD(void) const;

	real computeDistanceTo(const Vector &point) const;

	bool findIntersection(const Vector &point0, const Vector &point1) const;
	bool findIntersection(const Vector &point0, const Vector &point1, Vector &intersection) const;
	bool findIntersection(const Vector &point0, const Vector &point1, Vector &intersection, real &t) const;
	bool findIntersection(const Vector &point0, const Vector &point1, real &t) const;
	bool findDirectedIntersection(const Vector &point0, const Vector &point1) const;
	bool findDirectedIntersection(const Vector &point0, const Vector &point1, Vector &intersection) const;
	bool findDirectedIntersection(const Vector &point0, const Vector &point1, Vector &intersection, real &t) const;
	bool findDirectedIntersection(const Vector &point0, const Vector &point1, real &t) const;

	const Vector project(const Vector& point) const;

	static real computeD(const Vector &normal, const Vector &point);

};

// ======================================================================
// Compute the plane d coefficient
//
// Return value:
//
//   Plane D coefficient
//
// Remarks:
//
//   d = -(ax + by + cz) = -(normal dot point)

inline real Plane::computeD(const Vector &norm, const Vector &point)
{
	return -norm.dot(point);
}

// ----------------------------------------------------------------------
/**
 * Construct a plane.
 * 
 * The default plane will be pointed down the position Z axis, and be located
 * at the origin.
 */

inline Plane::Plane(void)
: normal(Vector::unitZ),
	d(CONST_REAL(0))
{
}

// ----------------------------------------------------------------------
/**
 * Construct a plane.
 * 
 * This routine constructs a plane with the specified normal and D-plane coefficient.
 * 
 * @param newNormal  [IN]  Normal for the plane
 * @param newD  [IN]  D-plane coefficient for the plane
 */

inline Plane::Plane(const Vector &newNormal, real newD)
: normal(newNormal),
	d(newD)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a plane.
 * 
 * This routine constructs a plane with the specified normal. The point
 * on the plane is used to calculate the D-plane coefficient.
 * 
 * @param newNormal  [IN]  Normal for the plane
 * @param point  [IN]  Point on the plane
 */

inline Plane::Plane(const Vector &newNormal, const Vector &point)
: normal(newNormal),
	d(computeD(newNormal, point))
{
}

// ----------------------------------------------------------------------
/**
 * Set a plane.
 * 
 * This routine sets the plane to have the specified normal and D-plane coefficient.
 * 
 * @param newNormal  [IN]  Normal for the plane
 * @param newD  [IN]  D-plane coefficient for the plane
 */

inline void Plane::set(const Vector &newNormal, real newD)
{
	normal = newNormal;
	d  = newD;
}

// ----------------------------------------------------------------------
/**
 * Set a plane.
 * 
 * This routine sets the plane to have the specified normal and D-plane coefficient.
 * 
 * @param newNormal  [IN]  Normal for the plane
 * @param point  [IN]  Point on the plane
 */

inline void Plane::set(const Vector& newNormal, const Vector& point)
{
	normal = newNormal;
	d      = computeD(normal, point);
}

// ----------------------------------------------------------------------
/**
 * Compute the signed distance from the point to the plane.
 * 
 * If the result is 0, the point is on the plane.  If the result is positive,
 * the point is on the front half-space of the plane.  If the result is
 * negative, the point is on the back half-space of the plane.
 * 
 * @param point  [IN]  Point to test against the plane
 * @return Signed distance from the point to the plane.
 */

inline real Plane::computeDistanceTo(const Vector &point) const
{
	return normal.dot(point) + d;
}

// ----------------------------------------------------------------------
/**
 * Get the plane's normal.
 * 
 * The normal will be a unit vector pointing orthogonal to the plane.
 * 
 * @return Normal for the plane
 */

inline const Vector &Plane::getNormal(void) const
{
	return normal;
}

// ----------------------------------------------------------------------
/**
 * Get the plane's D coefficient.
 * 
 * The D coefficient if the value that makes the plane equation true,
 * given XYZ as the plane's normal:  X * x + Y * y + Z * z + D = 0.
 * This value also represents the minimum distance from the origin to
 * the plane.
 * 
 * @return The D coefficient for the plane
 */

inline const real Plane::getD(void) const
{
	return d;
}

// ----------------------------------------------------------------------
/**
 * Find point projected onto the plane.
 * 
 * @return The projected point onto the plane.
 */

inline const Vector Plane::project(const Vector &point) const
{
	return point - (normal * computeDistanceTo(point));
}

// ----------------------------------------------------------------------
/**
 * Set the value of this plane to be that of ther other with the specified transformation applied.
 * 
 * @param other the other plane
 * @param trans the transformation to be applied.
 */

inline void Plane::set (const Plane & other, const Transform & trans)
{
	normal  = other.normal;
	d		= other.d;

	transform (trans);
}


// ======================================================================

#endif
