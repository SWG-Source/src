// ======================================================================
//
// Quaternion.h
// Portions Copyright 1999, Bootprint Entertainment
// Portions Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_Quaternion_H
#define INCLUDED_Quaternion_H

// ======================================================================

class Transform;
class Vector;

// ======================================================================

class Quaternion
{
	friend class Iff;

public:

	static const Quaternion identity;

public:

	real w;
	real x;
	real y;
	real z;

public:

	Quaternion(void);
	explicit Quaternion(const Transform &transform);
	Quaternion(real angle, const Vector &vector);
	Quaternion(real newW, real newX, real newY, real newZ);
	~Quaternion(void);

	bool               operator ==(const Quaternion &rhs);
	bool               operator !=(const Quaternion &rhs);

	const Quaternion   operator -(void) const;
	Quaternion        &operator +=(const Quaternion &rhs);
	Quaternion        &operator -=(const Quaternion &rhs);
	Quaternion        &operator *=(const Quaternion &rhs);

	const Quaternion   operator +(const Quaternion &rhs) const;
	const Quaternion   operator -(const Quaternion &rhs) const;
	const Quaternion   operator *(const Quaternion &rhs) const;

	void               getTransform(Transform *transform) const;
	void               getTransformPreserveTranslation(Transform *transform) const;
	real               getMagnitudeSquared(void) const;

	const Quaternion   slerp(const Quaternion &other, real fractionOfOther) const;

	void               normalize(void);

	Quaternion         getComplexConjugate() const;

	void               debugDump() const;
	real dot(Quaternion const & rhs) const;
};

// ======================================================================
/**
 * Retrieve the complex conjugate of this Quaternion instance.
 *
 * When a Quaternion has a unit length Vector, the complex conjugate
 * is equivalent to the inverse.  This is similar to a pure rotation
 * matrix, which has a simple inverse equivalent to the transpose of
 * the matrix.
 *
 * @return  
 */

inline Quaternion Quaternion::getComplexConjugate() const
{
	return Quaternion(w, -x, -y, -z);
}

// ----------------------------------------------------------------------

inline bool Quaternion::operator ==(const Quaternion &rhs)
{
	return (w == rhs.w && x == rhs.x && y == rhs.y && z == rhs.z);
}

// ----------------------------------------------------------------------

inline bool Quaternion::operator !=(const Quaternion &rhs)
{
	return !(*this == rhs);
}


inline real Quaternion::dot(Quaternion const & rhs) const
{
	return w * rhs.w + x * rhs.x + y * rhs.y + z * rhs.z;
}

// ======================================================================

#endif
