// ======================================================================
//
// Plane.cpp
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Plane.h"
#include "sharedMath/Transform.h"

// ======================================================================
// Construct a plane given three non-colinear points
//
// Remarks:
//
//   The front half of the plane is the side from which the vertices would
//   be specified in a clockwise order.

Plane::Plane(
	const Vector &point0,  // [IN]  First point on the plane
	const Vector &point1,  // [IN]  Second point on the plane
	const Vector &point2   // [IN]  Third point on the plane
	)
: normal(),
	d(CONST_REAL(0))
{
	set(point0, point1, point2);
}

// ----------------------------------------------------------------------
/**
 * Define a plane given three non-colinear points.
 * 
 * The front half of the plane is the side from which the vertices would
 * be specified in a clockwise order.
 * 
 * @param point0  [IN]  First point on the plane
 * @param point1  [IN]  Second point on the plane
 * @param point2  [IN]  Third point on the plane
 */

void Plane::set(const Vector &point0, const Vector &point1, const Vector &point2)
{
	// calculate the new normal direction
	normal = (point0 - point2).cross(point1 - point0);

	// normalize the normal
	if (!normal.normalize())
	{
		normal = Vector::unitZ;
		DEBUG_WARNING(true, ("Plane::calculate could not normalize vector"));
	}

	// compute the plane D coefficient
	d = -normal.dot(point0);
}

// ----------------------------------------------------------------------
/**
 * Find the intersection between a line segment and the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @return True if the line segment intersects the plane, otherwise false.
 */

bool Plane::findIntersection(const Vector &point0, const Vector &point1) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make sure the endpoints span the plane
	return (t0 * t1) < 0; // if either is zero, the point is on the plane
}

// ----------------------------------------------------------------------
/**
 * Find the intersection between a line segment and the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @param intersection  [OUT] Intersection of the point and the plane
 * @return True if the line segment intersects the plane, otherwise false.
 */

bool Plane::findIntersection(const Vector &point0, const Vector &point1, Vector &intersection) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make sure the endpoints span the plane
	if ((t0 * t1) > CONST_REAL(0))
		return false;

	if (t0 == t1) // both zero
	{
		intersection = point1;
		return true;
	}

	float abst = t0 / (t0 - t1); // safe since sign of t0 is always opposite t1
	intersection = Vector::linearInterpolate(point0, point1, abst);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Find the intersection between a line segment and the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @param intersection  [OUT] Intersection of the point and the plane
 * @param t  [OUT] parameterized t from 0..1
 * @return True if the line segment intersects the plane, otherwise false.
 */

bool Plane::findIntersection(const Vector &point0, const Vector &point1, Vector &intersection, float &t) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make sure the endpoints span the plane
	if ((t0 * t1) > CONST_REAL(0))
		return false;

	if (t0 == t1)	 // both zero
	{
		intersection = point0;
		t=0;
		return true; 
	}

	float abst = t0 / (t0 - t1); // safe since sign of t0 is always opposite t1
	intersection = Vector::linearInterpolate(point0, point1, abst);

	t = abst;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Find the intersection between a line segment and the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @param intersection  [OUT] Intersection of the point and the plane
 * @param t  [OUT] parameterized t from 0..1
 * @return True if the line segment intersects the plane, otherwise false.
 */

bool Plane::findIntersection(const Vector &point0, const Vector &point1, float &t) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make sure the endpoints span the plane
	if ((t0 * t1) > CONST_REAL(0))
		return false;

	if (t0 == t1)	 // both zero
	{
		t=0;
		return true; 
	}

	t = t0 / (t0 - t1); // safe since sign of t0 is always opposite t1

	return true;
}

// ----------------------------------------------------------------------
/**
 * Find the directed intersection between a line segment and the plane.
 * 
 * This routine will only detect the intersection if point0
 * is on the front side of the plane, and point1 is on the
 * back side of the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @return True if the line segment intersects the plane from front-to-rear, otherwise false.
 */

bool Plane::findDirectedIntersection(const Vector &point0, const Vector &point1) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make t0 is on the front side of the plane and t1 is on the back side of the plane
	return !((t0 < CONST_REAL(0) || t1 > CONST_REAL(0)));
}

// ----------------------------------------------------------------------
/**
 * Find the directed intersection between a line segment and the plane.
 * 
 * This routine will only detect the intersection if point0
 * is on the front side of the plane, and point1 is on the
 * back side of the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @param intersection  [OUT] Intersection of the point and the plane (may be nullptr)
 * @param t  [OUT] parameterized t from 0..1
 * @return True if the line segment intersects the plane from front-to-rear, otherwise false.
 */

bool Plane::findDirectedIntersection(const Vector &point0, const Vector &point1, Vector &intersection) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make t0 is on the front side of the plane and t1 is on the back side of the plane
	if (t0 < CONST_REAL(0) || t1 > CONST_REAL(0))
		return false;

	if (t0 == t1)	 // both zero
	{
		intersection = point1;
		return true; 
	}

	// solve parametric equation to find the intersection point
	float abst = t0 / (t0 - t1); // safe sine sign of t0 is always opposite t1
	intersection = Vector::linearInterpolate(point0, point1, abst);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Find the directed intersection between a line segment and the plane.
 * 
 * This routine will only detect the intersection if point0
 * is on the front side of the plane, and point1 is on the
 * back side of the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @param intersection  [OUT] Intersection of the point and the plane (may be nullptr)
 * @param t  [OUT] parameterized t from 0..1
 * @return True if the line segment intersects the plane from front-to-rear, otherwise false.
 */

bool Plane::findDirectedIntersection(const Vector &point0, const Vector &point1, Vector &intersection, real &t) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make t0 is on the front side of the plane and t1 is on the back side of the plane
	if (t0 < CONST_REAL(0) || t1 > CONST_REAL(0))
		return false;

	if (t0 == t1)	 // both zero
	{
		intersection = point0;
		t=0;
		return true; 
	}

	// solve parametric equation to find the intersection point
	float abst = t0 / (t0 - t1); // safe sine sign of t0 is always opposite t1
	t = abst;
	intersection = Vector::linearInterpolate(point0, point1, abst);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Find the directed intersection between a line segment and the plane.
 * 
 * This routine will only detect the intersection if point0
 * is on the front side of the plane, and point1 is on the
 * back side of the plane.
 * 
 * If the line segment does intersect the plane, and the intersection
 * pointer is non-nullptr, then intersection will be set to the point on
 * the line segment that crosses the plane.
 * 
 * @param point0  [IN] Start of the line segment
 * @param point1  [IN]  End of the line segment
 * @param intersection  [OUT] Intersection of the point and the plane (may be nullptr)
 * @param t  [OUT] parameterized t from 0..1
 * @return True if the line segment intersects the plane from front-to-rear, otherwise false.
 */

bool Plane::findDirectedIntersection(const Vector &point0, const Vector &point1, real &t) const
{
	const real t0(computeDistanceTo(point0));
	const real t1(computeDistanceTo(point1));

	// check to make t0 is on the front side of the plane and t1 is on the back side of the plane
	if (t0 < CONST_REAL(0) || t1 > CONST_REAL(0))
		return false;

	if (t0 == t1)	 // both zero
	{
		t=0;
		return true; 
	}

	// solve parametric equation to find the intersection point
	float abst = t0 / (t0 - t1); // safe sine sign of t0 is always opposite t1
	t = abst;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Transform the plane by the specified transformation
 * 
 * @param trans the transformation to apply.
 */

void Plane::transform (const Transform & trans)
{
	Vector const & p = trans.rotateTranslate_l2p (normal * -d);
	normal		= trans.rotate_l2p (normal);
	d			= computeD (normal, p);
}

// ----------------------------------------------------------------------
/**
 * Transform the plane by the specified transformation
 * 
 * @param trans the transformation to apply.
 */

void Plane::transform_p2l(const Transform &trans)
{
	Vector const & p = trans.rotateTranslate_p2l(normal * -d);
	normal		= trans.rotate_p2l(normal);
	d			= computeD (normal, p);
}

// ======================================================================
