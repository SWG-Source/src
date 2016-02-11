// ======================================================================
//
// Quaternion.cpp
// Portions Copyright 1999, Bootprint Entertainment
// Portions Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Quaternion.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"


// ======================================================================

namespace QuaternionNamespace
{
	float const s_quatEpsilon = 1.19209e-007f;
	float const s_quatEqualityEpsilon = 1e-027f;
}

using namespace QuaternionNamespace;

// ======================================================================

const Quaternion Quaternion::identity;

// ======================================================================

Quaternion::Quaternion(void) :
	w(1.0f),
	x(0.0f),
	y(0.0f),
	z(0.0f)
{
}

// ----------------------------------------------------------------------
/**
 * construct a quaternion representing the rotational orientation specified
 * by the given transform.
 */

Quaternion::Quaternion(const Transform &transform) :
	w(1.0f),
	x(0.0f),
	y(0.0f),
	z(0.0f)
{
	const float trace = transform.matrix[0][0] + transform.matrix[1][1] + transform.matrix[2][2] + 1.0f;
	if (trace >= 2.0f)
	{
		const float sqrtTrace = sqrt(trace);
		w = sqrtTrace * 0.5f;

		const float d = 0.5f / sqrtTrace;
		x = (transform.matrix[2][1] - transform.matrix[1][2]) * d;
		y = (transform.matrix[0][2] - transform.matrix[2][0]) * d;
		z = (transform.matrix[1][0] - transform.matrix[0][1]) * d;
	}
	else
	{
		int i = 0, j = 1, k = 2;
		if (transform.matrix[1][1] > transform.matrix[i][i])
			i = 1, j = 2, k = 0;
		if (transform.matrix[2][2] > transform.matrix[i][i])
			i = 2, j = 0, k = 1;

		// super hack for efficiency
		float *v = &x;
		v[i] = sqrt(((transform.matrix[i][i] - transform.matrix[j][j]) - transform.matrix[k][k]) + 1.0f) * 0.5f; //lint !e662 !e661
		const float d = 1.0f / (4.0f * v[i]); //lint !e662 !e661
		v[j] = (transform.matrix[j][i] + transform.matrix[i][j]) * d;
		v[k] = (transform.matrix[k][i] + transform.matrix[i][k]) * d; //lint !e661
		w    = (transform.matrix[k][j] - transform.matrix[j][k]) * d;
	}
}

// ----------------------------------------------------------------------
/**
 * construct a quaternion representing the orientation specified by spinning
 * 'angle' number of radians around unit vector 'vector'.
 * 
 * Make sure 'vector' is normalized.  This routine will not normalize it
 * for you.
 * 
 * @param angle  [IN] angle to spin around vector (in radians)
 * @param vector  [IN] vector around which angle is spun (must be normalized)
 */

Quaternion::Quaternion(float angle, const Vector &vector) :
	w(0.0f),
	x(0.0f),
	y(0.0f),
	z(0.0f)
{
	// -TRF- do a DEBUG_FATAL check on magnitude to ensure it is nearly 1.0

	const float halfAngle    = 0.5f * angle;
	const float sinHalfAngle = sin(halfAngle);

	w = cos(halfAngle);
	x = vector.x * sinHalfAngle;
	y = vector.y * sinHalfAngle;
	z = vector.z * sinHalfAngle;
}

// ----------------------------------------------------------------------

Quaternion::Quaternion(float newW, float newX, float newY, float newZ) :
	w(newW),
	x(newX),
	y(newY),
	z(newZ)
{
}

// ----------------------------------------------------------------------

Quaternion::~Quaternion(void)
{
}

// ----------------------------------------------------------------------

void Quaternion::getTransform(Transform *transform) const
{
	NOT_NULL(transform);

	getTransformPreserveTranslation(transform);
	transform->setPosition_p(Vector::zero);
}

// ----------------------------------------------------------------------

void Quaternion::getTransformPreserveTranslation(Transform *transform) const
{
	DEBUG_FATAL(!transform, ("nullptr transform arg"));

	if ((w + s_quatEqualityEpsilon) < 1.f) 
	{
		const float yyTimes2 = y * y * 2.0f;
		const float zzTimes2 = z * z * 2.0f;
		const float xyTimes2 = x * y * 2.0f;
		const float wzTimes2 = w * z * 2.0f;
		const float xzTimes2 = x * z * 2.0f;
		const float wyTimes2 = w * y * 2.0f;
		
		transform->matrix[0][0] = (1.0f - yyTimes2) - zzTimes2;
		transform->matrix[0][1] = xyTimes2 - wzTimes2;
		transform->matrix[0][2] = xzTimes2 + wyTimes2;
		
		const float xxTimes2 = x * x * 2.0f;
		const float yzTimes2 = y * z * 2.0f;
		const float wxTimes2 = w * x * 2.0f;
		
		transform->matrix[1][0] = xyTimes2 + wzTimes2;
		transform->matrix[1][1] = (1.0f - xxTimes2) - zzTimes2;
		transform->matrix[1][2] = yzTimes2 - wxTimes2;
		
		transform->matrix[2][0] = xzTimes2 - wyTimes2;
		transform->matrix[2][1] = yzTimes2 + wxTimes2;
		transform->matrix[2][2] = (1.0f - xxTimes2) - yyTimes2;
	}
	else
	{
		transform->resetRotate_l2p();
	}
}

// ----------------------------------------------------------------------

const Quaternion Quaternion::operator -(void) const
{
	return Quaternion(-w, -x, -y, -z);
}

// ----------------------------------------------------------------------

Quaternion &Quaternion::operator +=(const Quaternion &rhs)
{
	w += rhs.w;
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

// ----------------------------------------------------------------------

Quaternion &Quaternion::operator -=(const Quaternion &rhs)
{
	w -= rhs.w;
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

// ----------------------------------------------------------------------

Quaternion &Quaternion::operator *=(const Quaternion &rhs)
{
	// not effective to define this here since we'd need to save all the values
	// as we computed them anyway.
	*this = Quaternion(*this) * rhs;
	return *this;
} //lint !e1762 // function could be const - huh? no it couldn't...

// ----------------------------------------------------------------------

float Quaternion::getMagnitudeSquared(void) const
{
	return w * w + x * x + y * y + z * z;
}

// ----------------------------------------------------------------------

void Quaternion::normalize(void)
{
	float reciprocalMag = 1.0f / sqrt( x * x + y * y + z * z + w * w );

	x *= reciprocalMag;
	y *= reciprocalMag;
	z *= reciprocalMag;
	w *= reciprocalMag;
}

// ----------------------------------------------------------------------
/**
 * perform spherical linear interpolation between this quaternion and
 * 'other' quaternion.
 * 
 * This routine performs a spherical linear interpolation between the
 * orientation represented by this quaternion and the orientation
 * represented by 'other' quaternion.  'fractionOfOther' specifies
 * the fraction of 'other' blended with this quaternion.  A fraction
 * of 0.0 indicates only this quaternion, whereas a fraction of 1.0
 * indicates only the 'other' quaternion.  Values in between represent
 * a spherical linear interpolation between the two quaternions.
 * 
 * Although not a strict requirement, 'fractionOfOther' typically should
 * be restricted to the range zero to one.
 */
const Quaternion Quaternion::slerp(const Quaternion & otherOriginal, float fractionOfOther) const
{
	// rls - check ensure interpolation using the shortest path around the "hypersphere."
	float const dotOriginal = dot(otherOriginal);
	Quaternion const otherClosest(dotOriginal < 0.0f ? -otherOriginal : otherOriginal);

	float const cosTheta = dot(otherClosest);
	if ((1.0f + cosTheta) > s_quatEpsilon)
	{
		float c1, c2;

		// usual case. this means sin theta has enough value.
		if ((1.0f - cosTheta) > s_quatEpsilon)
		{
			// usual
			float const theta =  acos(cosTheta);
			float const ooSinTheta = 1.0f / sin(theta); // rls - multiply instead of divide.
			float const fractionTimesTheta = fractionOfOther * theta;
			c1 = sin(theta - fractionTimesTheta) * ooSinTheta;
			c2 = sin(fractionTimesTheta) * ooSinTheta;
		}
		else
		{
			// ends very close
			c1 =  1.0f - fractionOfOther;
			c2 = fractionOfOther;
		}

		return Quaternion(
			c1 * w + c2 * otherClosest.w,
			c1 * x + c2 * otherClosest.x,
			c1 * y + c2 * otherClosest.y,
			c1 * z + c2 * otherClosest.z
			);
	}

	// ends nearly opposite
	float const fractionTimesTheta = PI * fractionOfOther;
	float const c1 = sin(PI_OVER_2 - fractionTimesTheta);
	float const c2 = sin(fractionTimesTheta);

	return Quaternion(
		c1 * w + c2 * z,
		c1 * x - c2 * y,
		c1 * y + c2 * x,
		c1 * z - c2 * w
		);
}


// ----------------------------------------------------------------------

const Quaternion Quaternion::operator +(const Quaternion &rhs) const
{
	return Quaternion(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
}

// ----------------------------------------------------------------------

const Quaternion Quaternion::operator -(const Quaternion &rhs) const
{
	return Quaternion(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
}

// ----------------------------------------------------------------------

const Quaternion Quaternion::operator *(const Quaternion &rhs) const
{
	// rls - do not multiply by identity quaternions.
	if ((rhs.w + s_quatEqualityEpsilon) >= 1.f) 
	{
		return *this; // return *this because the other is an identity quaternion.
	}
	else if ((w + s_quatEqualityEpsilon) >= 1.f) 
	{
		return rhs; // return rhs because this quaternion is an identity quaternion.
	}

	// Equation from CRC Concise Encyclopedia of Mathematics, p 1494, equations 24 and 25
	//
	// Assume quaternion of form (a1,A) = a1 + a2*i + a3*j + a4*k
	//        (that is, A = [a2 a3 a4]T)
	// then (s1,V1) * (s2,V2) = (s1*s2 - V1 <dot> V2, s1*V2 + s2*V1 + V1 <cross> V2)
	//        where <dot>   = dot product binary operator and
	//              <cross> = cross product binary operator
	//
	// lhs * rhs
	//
	//   w = w * rhs.w - (x * rhs.x + y * rhs.y + z * rhs.z)
	//   x = w * rhs.x + rhs.w * x + (y * rhs.z - z * rhs.y)
	//   y = w * rhs.y + rhs.w * y + (z * rhs.x - x * rhs.z)
	//   z = w * rhs.z + rhs.w * z + (x * rhs.y - y * rhs.x)

	return Quaternion(
		w * rhs.w - (x * rhs.x + y * rhs.y + z * rhs.z),
		w * rhs.x + rhs.w * x + (y * rhs.z - z * rhs.y),
		w * rhs.y + rhs.w * y + (z * rhs.x - x * rhs.z),
		w * rhs.z + rhs.w * z + (x * rhs.y - y * rhs.x)
		);
}

// ----------------------------------------------------------------------

void Quaternion::debugDump() const
{
	DEBUG_REPORT_LOG(true, ("[w=%g,x=%g,y=%g,z=%g]\n", w, x, y, z));
}

// ======================================================================
