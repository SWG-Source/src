// ======================================================================
//
// Vector.h
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2001 Sony Online Entertainment.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Vector_H
#define INCLUDED_Vector_H

// ======================================================================
#include <cmath>

class Vector
{
public:

	// Vector 1,0,0
	static const Vector unitX;

	// Vector 0,1,0
	static const Vector unitY;

	// Vector 0,0,1
	static const Vector unitZ;

	// Vector -1,0,0
	static const Vector negativeUnitX;

	// Vector 0,-1,0
	static const Vector negativeUnitY;

	// Vector 0,0,-1
	static const Vector negativeUnitZ;

	// Vector 0,0,0
	static const Vector zero;

	// Vector 1,1,1
	static const Vector xyz111;

	// Vector max, max, max
	static const Vector maxXYZ;

	// Vector -maxXYZ
	static const Vector negativeMaxXYZ;

	// minimum vector magnitude to normalize
	static const real   NORMALIZE_THRESHOLD;

	// A vector is normalized if its magnitude is within NORMALIZED_EPSILON of 1.
	static const real   NORMALIZED_EPSILON;

public:

	// x-component of the 3d vector
	real x;  //lint !e1925 // Public data member

	// y-component of the 3d vector
	real y;  //lint !e1925 // Public data member

	// z-component of the 3d vector
	real z;  //lint !e1925 // Public data member

public:

	Vector(void);
	Vector(real newX, real newY, real newZ);

	void                 debugPrint(const char *header) const;

	void                 set(real newX, real newY, real newZ);
	void                 makeZero(void);

	bool                 normalize(void);
	bool                 approximateNormalize(void);
	bool                 isNormalized(void) const;

	const Vector         findClosestPointOnLine(const Vector &line0, const Vector &line1) const;
	const Vector         findClosestPointOnLine(const Vector &line0, const Vector &line1, real *t) const;
	const Vector         findClosestPointOnLineSegment(const Vector & startPoint, const Vector & endPoint) const;
	real                 distanceToLine(const Vector &line0, const Vector &line1) const;
	real                 distanceToLineSegment(const Vector &line0, const Vector &line1) const;

	real                 theta(void) const;
	real                 phi(void) const;

	real                 dot(const Vector &vector) const;
	const Vector         cross(const Vector &rhs) const;

	real                 magnitudeSquared(void) const;
	real                 approximateMagnitude(void) const;
	real                 magnitude(void) const;
	real                 magnitudeBetween(const Vector &vector) const;
	real                 magnitudeBetweenSquared(const Vector &vector) const;
	real                 magnitudeXYBetween(const Vector &vector) const;
	real                 magnitudeXYBetweenSquared(const Vector &vector) const;
	real                 magnitudeXZBetween(const Vector &vector) const;
	real                 magnitudeXZBetweenSquared(const Vector &vector) const;
	real                 magnitudeYZBetween(const Vector &vector) const;
	real                 magnitudeYZBetweenSquared(const Vector &vector) const;

	const Vector         operator -(void) const;

	Vector              &operator -=(const Vector &rhs);
	Vector              &operator +=(const Vector &rhs);
	Vector              &operator /=(real scalar);
	Vector              &operator *=(real scalar);

	const Vector         operator +(const Vector &rhs) const;
	const Vector         operator -(const Vector &rhs) const;
	const Vector         operator *(real scalar) const;
	const Vector         operator /(real scalar) const;
	friend const Vector  operator *(real scalar, const Vector &vector);

	bool                 operator ==(const Vector &rhs) const;
	bool                 operator !=(const Vector &rhs) const;
	bool                 withinEpsilon(const Vector &rhs, float epsilon) const;

	const Vector         reflectIncoming(const Vector &incident) const;
	const Vector         reflectOutgoing(const Vector &incident) const;

	// const Vector      refract(const Vector &normal, real n1, real n2);

	bool                 inPolygon (const Vector& v0, const Vector& v1, const Vector& v2) const;
	bool                 inPolygon (const std::vector<Vector> &convexPolygonVertices) const;

public:

	static const Vector  midpoint(const Vector &vector1, const Vector &vector2);
	static const Vector  linearInterpolate(const Vector &begin, const Vector &end, real t);

	static const Vector  randomUnit(void);
	static const Vector  randomCube(real halfSideLength=CONST_REAL(1));

	static const Vector perpendicular(Vector const & direction);
};

// ======================================================================
// Construct a vector
//
// Remarks:
//
//   Initializes the components to 0.

inline Vector::Vector(void)
: x(CONST_REAL(0)), y(CONST_REAL(0)), z(CONST_REAL(0))
{
}

// ----------------------------------------------------------------------
/**
 * Construct a vector.
 * 
 * Initializes the components to the specified values
 * 
 * @param newX  Value for the X component
 * @param newY  Value for the Y component
 * @param newZ  Value for the Z component
 */

inline Vector::Vector(real newX, real newY, real newZ)
: x(newX), y(newY), z(newZ)
{
}

// ----------------------------------------------------------------------
/**
 * Set the vector components to new values.
 * 
 * @param newX  Value for the X component
 * @param newY  Value for the Y component
 * @param newZ  Value for the Z component
 */

inline void Vector::set(real newX, real newY, real newZ)
{
	x = newX;
	y = newY;
	z = newZ;
}

// ----------------------------------------------------------------------
/**
 * Set all the vector components to zero.
 */

inline void Vector::makeZero(void)
{
	x = CONST_REAL(0);
	y = CONST_REAL(0);
	z = CONST_REAL(0);
}

// ----------------------------------------------------------------------
/**
 * Return the rotation of the vector around the Y plane.
 * 
 * The result is undefined if both the x and z values of the vector are zero.
 * 
 * This routine uses atan2() so it is not particularly fast.
 * 
 * @return The rotation of the vector around the Y plane
 */

inline real Vector::theta(void) const
{
	return atan2(x, z);
}

// ----------------------------------------------------------------------
/**
 * Calculate the angle of the vector from the X-Z plane.
 * 
 * This routine uses sqrt() and atan2() so it is not particularly fast.
 * 
 * @return The angle of the vector from the X-Z plane
 */

inline real Vector::phi(void) const
{
	return atan2(-y, sqrt(sqr(x) + sqr(z)));
}

// ----------------------------------------------------------------------
/**
 * Compute the dot product between this vector and another vector.
 * 
 * The dot product value is equal to the cosine of the angle between
 * the two vectors multiplied by the sum of the lengths of the vectors.
 * 
 * @param vector  Vector to compute the dot product against
 */

inline real Vector::dot(const Vector &vec) const
{
	return (x * vec.x) + (y * vec.y) + (z * vec.z);
}

// ----------------------------------------------------------------------
/**
 * Calculate the square of the magnitude of this vector.
 * 
 * This routine is much faster than magnitude().
 * 
 * @return The square of the magnitude of the vector
 * @see Vector::magnitude()
 */

inline real Vector::magnitudeSquared(void) const
{
	return (sqr(x) + sqr(y) + sqr(z));
}

// ----------------------------------------------------------------------
/**
 * Calculate the approximate magnitude of this vector.
 * 
 * The implementation of this routine has +/- 8% error.
 * 
 * @return The approximate magnitude of the vector
 */


inline real Vector::approximateMagnitude(void) const
{
	real minc = std::abs(x);
	real midc = std::abs(y);
	real maxc = std::abs(z);

	// sort the vectors
	// we do our own swapping to avoid heavy-weight includes in such a low-level class
	if (midc < minc)
	{
		const real temp = midc;
		midc = minc;
		minc = temp;
	}

	if (maxc < minc)
	{
		const real temp = maxc;
		maxc = minc;
		minc = temp;
	}

	if (maxc < midc)
	{
		const real temp = maxc;
		maxc = midc;
		midc = temp;
	}

	return (maxc + CONST_REAL(11.0f / 32.0f) * midc + CONST_REAL(0.25f) * minc);
}

// ----------------------------------------------------------------------
/**
 * Calculate the magnitude of this vector.
 * 
 * This routine is slow because it requires a square root operation.
 * 
 * @return The magnitude of the vector
 * @see Vector::magnitudeSquared()
 */

inline real Vector::magnitude(void) const
{
	return sqrt(magnitudeSquared());
}

// ----------------------------------------------------------------------
/**
 * Calculate the square of the magnitude of the vector between this vector and the specified vector.
 * 
 * This routine is much faster than magnitudeBetween().
 * 
 * @param vector  The other endpoint of the delta vector
 * @return The square of the magnitude of the delta vector
 * @see Vector::magnitudeBetween()
 */

inline real Vector::magnitudeBetweenSquared(const Vector &vec) const
{
	return (sqr(x - vec.x) + sqr(y - vec.y) + sqr(z - vec.z));
}

// ----------------------------------------------------------------------
/**
 * Calculate the magnitude of the vector between this vector and the specified vector.
 * 
 * This routine is much slower than magnitudeBetweenSquared().
 * 
 * @param vector  The other endpoint of the delta vector
 * @return The magnitude of the delta vector
 * @see Vector::magnitudeBetweenSquared()
 */

inline real Vector::magnitudeBetween(const Vector &vec) const
{
	return sqrt(magnitudeBetweenSquared(vec));
}

// ----------------------------------------------------------------------
/**
* Calculate the square of the magnitude of the vector between this vector and the specified vector.
* 
* This routine is much faster than magnitudeBetween().
* 
* @param vector  The other endpoint of the delta vector
* @return The square of the magnitude of the delta vector
* @see Vector::magnitudeBetween()
*/

inline real Vector::magnitudeXYBetweenSquared(const Vector &vec) const
{
	return (sqr(x - vec.x) + sqr(y - vec.y));
}

// ----------------------------------------------------------------------
/**
* Calculate the magnitude of the vector between this vector and the specified vector.
* 
* This routine is much slower than magnitudeBetweenSquared().
* 
* @param vector  The other endpoint of the delta vector
* @return The magnitude of the delta vector
* @see Vector::magnitudeBetweenSquared()
*/

inline real Vector::magnitudeXYBetween(const Vector &vec) const
{
	return sqrt(magnitudeXYBetweenSquared(vec));
}

// ----------------------------------------------------------------------
/**
* Calculate the square of the magnitude of the vector between this vector and the specified vector.
* 
* This routine is much faster than magnitudeBetween().
* 
* @param vector  The other endpoint of the delta vector
* @return The square of the magnitude of the delta vector
* @see Vector::magnitudeBetween()
*/

inline real Vector::magnitudeXZBetweenSquared(const Vector &vec) const
{
	return (sqr(x - vec.x) + sqr(z - vec.z));
}

// ----------------------------------------------------------------------
/**
* Calculate the magnitude of the vector between this vector and the specified vector.
* 
* This routine is much slower than magnitudeBetweenSquared().
* 
* @param vector  The other endpoint of the delta vector
* @return The magnitude of the delta vector
* @see Vector::magnitudeBetweenSquared()
*/

inline real Vector::magnitudeXZBetween(const Vector &vec) const
{
	return sqrt(magnitudeXZBetweenSquared(vec));
}

// ----------------------------------------------------------------------
/**
* Calculate the square of the magnitude of the vector between this vector and the specified vector.
* 
* This routine is much faster than magnitudeBetween().
* 
* @param vector  The other endpoint of the delta vector
* @return The square of the magnitude of the delta vector
* @see Vector::magnitudeBetween()
*/

inline real Vector::magnitudeYZBetweenSquared(const Vector &vec) const
{
	return (sqr(y - vec.y) + sqr(z - vec.z));
}

// ----------------------------------------------------------------------
/**
* Calculate the magnitude of the vector between this vector and the specified vector.
* 
* This routine is much slower than magnitudeBetweenSquared().
* 
* @param vector  The other endpoint of the delta vector
* @return The magnitude of the delta vector
* @see Vector::magnitudeBetweenSquared()
*/

inline real Vector::magnitudeYZBetween(const Vector &vec) const
{
	return sqrt(magnitudeYZBetweenSquared(vec));
}

// ----------------------------------------------------------------------
/**
 * Reverse the direction of the vector.
 * 
 * This routine simple negates each component of the vector
 */

inline const Vector Vector::operator -(void) const
{
	return Vector(-x, -y, -z);
}

// ----------------------------------------------------------------------
/**
 * Subtract a vector from this vector and store the result back in this vector.
 * 
 * This is the basic obvious -= operator overloaded for vectors
 * 
 * @param rhs  The vector to subtract from this vector
 * @return A reference to this modified vector
 */

inline Vector &Vector::operator -=(const Vector &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Add a vector from this vector and store the result back in this vector.
 * 
 * This is the basic obvious += operator overloaded for vectors.
 * 
 * @param rhs  The vector to add to this vector
 * @return A reference to this modified vector
 */

inline Vector &Vector::operator +=(const Vector &rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Multiply this vector by a scalar.
 * 
 * This is the basic obvious *= operator overloaded for vectors and scalars.
 * 
 * @param scalar  The vector to subtract from this vector
 * @return A reference to this modified vector
 */

inline Vector &Vector::operator *=(real scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Divide this vector by a scalar.
 * 
 * This is the basic obvious /= operator overloaded for vectors and scalars.
 * 
 * @param scalar  The vector to subtract from this vector
 * @return A reference to this modified vector
 */

inline Vector &Vector::operator /=(real scalar)
{
	*this *= (CONST_REAL(1.0) / scalar);
	return *this;
}

// ----------------------------------------------------------------------
/**
 * Calculate the cross product between two vectors.
 * 
 * This routine returns a temporary.
 * 
 * Cross products are not communitive.
 * 
 * @param rhs  The right-hand size of the expression
 * @return A vector that is the result of the cross product of the two vectors.
 */

inline const Vector Vector::cross(const Vector &rhs) const
{
	return Vector(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

// ----------------------------------------------------------------------
/**
 * Add two vectors.
 * 
 * This routine returns a temporary.
 * 
 * @param rhs  The right-hand size of the expression
 * @return A vector that is the sum of the two arguments.
 */

inline const Vector Vector::operator +(const Vector &rhs) const
{
	return Vector(x + rhs.x, y + rhs.y, z + rhs.z);
}

// ----------------------------------------------------------------------
/**
 * Subtract two vectors.
 * 
 * This routine returns a temporary.
 * 
 * @param rhs  The right-hand size of the expression
 * @return A vector that is the result of the left-hand-side minus the right-hand-side
 */

inline const Vector Vector::operator -(const Vector &rhs) const
{
	return Vector(x - rhs.x, y - rhs.y, z - rhs.z);
}

// ----------------------------------------------------------------------
/**
 * Multiply a vector by a scalar.
 * 
 * This routine returns a temporary.
 * 
 * @param scalar  The scalar to multiply by
 * @return The source vector multiplied by the scalar
 */

inline const Vector Vector::operator *(real scalar) const
{
	return Vector(x * scalar, y * scalar, z * scalar);
}

// ----------------------------------------------------------------------
/**
 * Divide a vector by a scalar.
 * 
 * This routine returns a temporary.
 * 
 * @param scalar  The scalar to multiply by
 * @return The source vector divided by the scalar
 */

inline const Vector Vector::operator /(real scalar) const
{
	const real multiplier(CONST_REAL(1.0) / scalar);
	return Vector(x * multiplier, y * multiplier, z * multiplier);
}

// ----------------------------------------------------------------------
/**
 * Multiply a vector by a scalar.
 * 
 * This routine returns a temporary.
 * 
 * @return The source vector multiplied by the scalar
 */

inline const Vector operator *(real scalar, const Vector &vec)
{
	return Vector(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

// ----------------------------------------------------------------------
/**
 * Test two vectors for equality.
 * 
 * Floating-point math make may two very similiar expressions end up slightly
 * different, thus showing inequality when the result is very, very close.
 * 
 * @param rhs  The right-hand size of the expression
 * @return True if the vectors are exactly equal, otherwise false.
 */

inline bool Vector::operator ==(const Vector &rhs) const
{
	return (x == rhs.x && y == rhs.y && z == rhs.z);  //lint !e777 // Testing floats for equality
}

// ----------------------------------------------------------------------
/**
 * Test two vectors for inequality.
 * 
 * Floating-point math make may two very similiar expressions end up slightly
 * different, thus showing inequality when the result is very, very close.
 * 
 * @param rhs  The right-hand size of the expression
 * @return True if the vectors are not equal, otherwise false.
 */

inline bool Vector::operator !=(const Vector &rhs) const
{
	return (x != rhs.x || y != rhs.y || z != rhs.z); //lint !e777 // Testing floats for equality
}

// ----------------------------------------------------------------------
/**
 * Reflect an outgoing vector around this normal.
 * 
 * This routine assumes that both this vector and the normal around which it
 * is being reflected have the same origin (the dot product of the normal and
 * the vertex is positive)
 * 
 * @param incident  Normal to reflect this vector around
 * @return The reflected vector
 */

inline const Vector Vector::reflectOutgoing(const Vector &incident) const
{
	const Vector &normal = *this;
	return normal * (2 * normal.dot(incident)) - incident;
}

// ----------------------------------------------------------------------
/**
 * Reflect an incoming vector around this normal.
 * 
 * This routine assumes that the vector terminates at the normal
 * (the dot product of the normal and the vertex is negative).
 * 
 * @param incident  Normal to reflect this vector around
 * @return The reflected vector
 */

inline const Vector Vector::reflectIncoming(const Vector &incident) const
{
	return reflectOutgoing(-incident);
}

// ----------------------------------------------------------------------
/**
 * Compute the midpoint of two vectors.
 * 
 * This routine just averages the three components separately.
 * 
 * @param vector1  First endpoint
 * @param vector2  Second endpoint
 */

inline const Vector Vector::midpoint(const Vector &vector1, const Vector &vector2)
{
	return Vector((vector1.x + vector2.x) * CONST_REAL(0.5), (vector1.y + vector2.y) * CONST_REAL(0.5), (vector1.z + vector2.z) * CONST_REAL(0.5));
}

// ----------------------------------------------------------------------
/**
 * Linearly interpolate between two vectors.
 * 
 * The time parameter should be between 0.0 and 1.0 inclusive in order to have
 * the result be between the two endpoints.  At time 0.0 the result will be
 * vector1, and at time 1.0 the result will be vector2.
 * 
 * @param vector1  Starting endpoint
 * @param vector2  Terminating endpoint
 * @param time   
 */

inline const Vector Vector::linearInterpolate(const Vector &vector1, const Vector &vector2, real time)
{
	return Vector(vector1.x + (vector2.x - vector1.x) * time, vector1.y + (vector2.y - vector1.y) * time, vector1.z + (vector2.z - vector1.z) * time);
}

// ======================================================================

#endif
