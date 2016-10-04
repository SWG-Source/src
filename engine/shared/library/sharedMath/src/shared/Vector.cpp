// ======================================================================
//
// Vector.cpp
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2001 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Vector.h"

#include "sharedRandom/Random.h"

#include <cmath>
#include <vector>

// ======================================================================

const Vector Vector::unitX(CONST_REAL(1), CONST_REAL(0), CONST_REAL(0));
const Vector Vector::unitY(CONST_REAL(0), CONST_REAL(1), CONST_REAL(0));
const Vector Vector::unitZ(CONST_REAL(0), CONST_REAL(0), CONST_REAL(1));

const Vector Vector::negativeUnitX(CONST_REAL(-1), CONST_REAL( 0), CONST_REAL( 0));
const Vector Vector::negativeUnitY(CONST_REAL( 0), CONST_REAL(-1), CONST_REAL( 0));
const Vector Vector::negativeUnitZ(CONST_REAL( 0), CONST_REAL( 0), CONST_REAL(-1));

const Vector Vector::zero(CONST_REAL(0), CONST_REAL(0), CONST_REAL(0));
const Vector Vector::xyz111(CONST_REAL(1), CONST_REAL(1), CONST_REAL(1));

const Vector Vector::maxXYZ(REAL_MAX, REAL_MAX, REAL_MAX);
const Vector Vector::negativeMaxXYZ(-REAL_MAX, -REAL_MAX, -REAL_MAX);

const real   Vector::NORMALIZE_THRESHOLD(CONST_REAL(0.00001));

const real   Vector::NORMALIZED_EPSILON(CONST_REAL(0.00001));

// If M is between 1-e and 1+e, then M^2 is between 1-2e+e^2 and 1+2e+e^2.

static const real   NORMALIZED_RANGE_SQUARED_MIN = (1.0f - (2.0f * Vector::NORMALIZED_EPSILON)) + (Vector::NORMALIZED_EPSILON * Vector::NORMALIZED_EPSILON);
static const real   NORMALIZED_RANGE_SQUARED_MAX = (1.0f + (2.0f * Vector::NORMALIZED_EPSILON)) + (Vector::NORMALIZED_EPSILON * Vector::NORMALIZED_EPSILON);

// ======================================================================
// Normalize a vector to a length of 1
//
// Return value:
//
//   True if the vector has been normalized, otherwise false.
//
// Remarks:
//
//   If the vector is too small, it cannot be normalized.

bool Vector::normalize(void)
{
	real mag = magnitude();

	if (mag < NORMALIZE_THRESHOLD)
		return false;

	*this /= mag;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Normalize a vector to a length of approximately 1.
 * 
 * If the vector is too small, it cannot be normalized.
 * 
 * @return True if the vector has been approximately normalized, otherwise false.
 */

bool Vector::approximateNormalize(void)
{
	real mag = approximateMagnitude();

	if (mag < NORMALIZE_THRESHOLD)
		return false;

	*this /= mag;

	return true;
}

// ----------------------------------------------------------------------
/**
 * Return true if a vector is of length 1 (within some tolerance)
 *
 * Vectors normalized with Vector::normalize() should always be of unit
 * length within tolerance, vectors normalized with Vector::approximate
 * Normalize will not.
 *
 * @return True if the vector's length is within NORMALIZED_TOLERANCE of 1
 */

bool Vector::isNormalized(void) const
{
	real mag2 = magnitudeSquared();

	return WithinRangeInclusiveInclusive(NORMALIZED_RANGE_SQUARED_MIN,mag2,NORMALIZED_RANGE_SQUARED_MAX);
}

// ----------------------------------------------------------------------
/**
 * Find the point on the specified line that is as close to this point as possible.
 * 
 * The line is treated as an infinite line, not a line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 * @param t  Parameteric time along the line that is closest to this vector
 */

const Vector Vector::findClosestPointOnLine(const Vector &line0, const Vector &line1, real *t) const
{
	DEBUG_FATAL(!t, ("t arg is nullptr"));

    NOT_NULL(t);

	Vector delta(line1 - line0);
	const real r  = (*this - line0).dot(delta) / delta.magnitudeSquared();
	*t = r;
	return line0 + delta * r;
}

// ----------------------------------------------------------------------
/**
 * Find the point on the specified line that is as close to this point as possible.
 * 
 * The line is treated as an infinite line, not a line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 */

const Vector Vector::findClosestPointOnLine(const Vector &line0, const Vector &line1) const
{
	real t;
	return findClosestPointOnLine(line0, line1, &t);
}

//-----------------------------------------------------------------------

const Vector Vector::findClosestPointOnLineSegment(const Vector & startPoint, const Vector & endPoint) const
{
	Vector delta(endPoint - startPoint);

	// if these vectors describe a point instead of a line-segment, return the startpoint
	// rather than returning an invalid vector
	const real deltaMagnitudeSquared = delta.magnitudeSquared();
	if(deltaMagnitudeSquared <NORMALIZE_THRESHOLD)
		return startPoint;
	
	const real r = clamp(CONST_REAL(0), (*this - startPoint).dot(delta) / deltaMagnitudeSquared, CONST_REAL(1));
	return startPoint + delta * r;
}

// ----------------------------------------------------------------------
/**
 * Calculate the distance from this point to the specified line.
 * 
 * The line is treated as an infinite line, not a line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 */

real Vector::distanceToLine(const Vector &line0, const Vector &line1) const
{
	return magnitudeBetween(findClosestPointOnLine(line0, line1));
}

// ----------------------------------------------------------------------
/**
 * Calculate the distance from this point to the specified line segment.
 * 
 * @param line0  First point on the line
 * @param line1  Second point on the line
 */

real Vector::distanceToLineSegment(const Vector &line0, const Vector &line1) const
{
	return magnitudeBetween(findClosestPointOnLineSegment(line0, line1));
}

// ----------------------------------------------------------------------
/**
 * Send this vector to the DebugPrint system.
 * 
 * The header parameter may be nullptr.
 * 
 * @param header  Header for the vector
 */

void Vector::debugPrint(const char *header) const
{
	if (header)
		DEBUG_REPORT_PRINT(true, ("%s: ", header));

	DEBUG_REPORT_PRINT(true, ("  %-8.2f %-8.2f %-8.2f\n", x, y, z));
}

// ----------------------------------------------------------------------
/**
 * Create a random unit vector.
 * 
 * This routine will return an evenly distributed random vector on the
 * unit sphere.
 */

const Vector Vector::randomUnit(void)
{
	// from the comp.graphics.algorithm FAQ
	const real lz = cos(Random::randomReal(0.0f, PI));
	const real t = Random::randomReal(0.f, PI_TIMES_2);
	const real r = sqrt(1.0f - sqr(lz));
	return Vector(r * cos(t), r * sin(t), lz);
}

// ----------------------------------------------------------------------
/**
 * Create a random vector.
 * 
 * The vector will be within the cube [ -halfSideLength .. halfSideLength ].
 * 
 * @param halfSideLength  Size of the cube
 */

const Vector Vector::randomCube(real halfSideLength)
{
	return Vector(Random::randomReal(-halfSideLength, halfSideLength), Random::randomReal(-halfSideLength, halfSideLength), Random::randomReal(-halfSideLength, halfSideLength));
}

// ----------------------------------------------------------------------
/**
 * Test whether a point lies within a triangle.
 * 
 * This should be on at least the plane for the test to work. See Plane::findIntersection ()
 */

namespace {
	// adapated from http://www.blackpawn.com/texts/pointinpoly/default.html
	inline bool sameSide(const Vector &p1, const Vector &p2, const Vector &a, const Vector &b)
	{
		Vector const ba(b - a);
		Vector const cp1(ba.cross(p1 - a));
		Vector const cp2(ba.cross(p2 - a));
		return cp1.dot(cp2) >= 0.0f;
	}
}

bool Vector::inPolygon (const Vector& v0, const Vector& v1, const Vector& v2) const
{
	return sameSide(*this, v0, v1, v2) && sameSide(*this, v1, v2, v0) && sameSide(*this, v2, v0, v1);
}

// ----------------------------------------------------------------------
/**
 * Test whether a point lies within a convex n-sided polygon.
 * 
 * This should be on at least the plane for the test to work. See Plane::findIntersection ()
 */

bool Vector::inPolygon(const std::vector<Vector> &convexPolygonVertices) const
{
	// @todo optimize this
	const uint numberOfConvexPolygonVertices = convexPolygonVertices.size();
	for (uint i = 1; i < numberOfConvexPolygonVertices-1; ++i)
		if (inPolygon(convexPolygonVertices[0], convexPolygonVertices[i], convexPolygonVertices[i+1]))
			return true;

	return false;
}

// ----------------------------------------------------------------------
/**
 * Determine if two vectors are within an epsilon distance.
 * The epsilon distance is tested for all three dimensions, not a
 * distance between the points.
 *
 * @param rhs The second vector to compare.
 * @param epsilon The epsilon distance.
 */
bool Vector::withinEpsilon(const Vector &rhs, float epsilon) const
{
	return (std::abs(x - rhs.x) < epsilon) && (std::abs(y - rhs.y) < epsilon) && (std::abs(z - rhs.z) < epsilon);
}


// ----------------------------------------------------------------------
/**
 * Find a direction which is perpendicular to the vector passed in.
 * NOTE: The result is NOT guaranteed to be normalized.
 */

const Vector Vector::perpendicular(Vector const & direction)
{
	// Measure the projection of "direction" onto each of the axes
	float const id = std::abs(direction.dot(Vector::unitX));
	float const jd = std::abs(direction.dot(Vector::unitY));
	float const kd = std::abs(direction.dot(Vector::unitZ));

	Vector result;

	if (id <= jd && id <= kd)
		// Projection onto i was the smallest
		result = direction.cross(Vector::unitX);
	else if (jd <= id && jd <= kd)
		// Projection onto j was the smallest
		result = direction.cross(Vector::unitY);
	else
		// Projection onto k was the smallest
		result = direction.cross(Vector::unitZ);

	result.normalize();

	return result;
}

// ======================================================================
