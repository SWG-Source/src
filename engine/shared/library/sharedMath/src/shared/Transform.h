// ======================================================================
//
// Transform.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Transform_H
#define INCLUDED_Transform_H

// ======================================================================

#include "sharedMath/Vector.h"

// ======================================================================

namespace DPVS
{
	class Matrix4x4;
}

class Transform
{
	friend class Direct3d8;
	friend class Direct3d9;
	friend class Iff;
	friend class MayaUtility;
	friend class OpenGL;
	friend class Quaternion;
	friend class SseMath;
	friend class RenderWorld;

public:

	enum InitializeFlag
	{
		IF_none
	};

public:

	static const Transform identity;

public:

	static void  install();

public:

	typedef real matrix_t[3][4];

	Transform(void);
	explicit Transform(InitializeFlag noInitialization);
	
	void           debugPrint(const char *header) const;

	void           reorthonormalize(void);
	void DLLEXPORT multiply(const Transform &lhs, const Transform &rhs);
	void           invert(const Transform &transform);

	void           move_l(const Vector &vec);
	void           move_p(const Vector &vec);

	void           yaw_l(real radians);
	void           pitch_l(real radians);
	void           roll_l(real radians);

	// set ypr of the matrix.
	void           ypr_l(real y, real p, real r);
	void           ypr_l(Vector const & ypr);

	void           resetRotate_l2p(void);
	void           resetRotateTranslate_l2p(void);

	const Vector   getPosition_p(void) const;
	void           setPosition_p(const Vector &vec);
	void           setPosition_p(real x, real y, real z);

	const Vector   getLocalFrameI_p(void) const;
	const Vector   getLocalFrameJ_p(void) const;
	const Vector   getLocalFrameK_p(void) const;
	void           setLocalFrameIJK_p(const Vector &i, const Vector &j, const Vector &k);
	void           setLocalFrameKJ_p(const Vector &k, const Vector &j);

	const Vector   getParentFrameI_l(void) const;
	const Vector   getParentFrameJ_l(void) const;
	const Vector   getParentFrameK_l(void) const;

	const Vector   rotate_l2p(const Vector &vec) const;
	const Vector   rotateTranslate_l2p(const Vector &vec) const;
	const Vector   rotate_p2l(const Vector &vec) const;
	const Vector   rotateTranslate_p2l(const Vector &vec) const;

	const Transform rotateTranslate_l2p(const Transform &t) const;
	const Transform rotateTranslate_p2l(const Transform &t) const;

	void           rotate_l2p(const Vector *source, Vector *result, int count) const;
	void           rotateTranslate_l2p(const Vector *source, Vector *result, int count) const;
	void           rotate_p2l(const Vector *source, Vector *result, int count) const;
	void           rotateTranslate_p2l(const Vector *source, Vector *result, int count) const;

	bool           operator== (const Transform& rhs) const;
	bool           operator!= (const Transform& rhs) const;
	bool           approximates(const Transform &rhs, float rotDelta=0.001f, float posEpsilon=0.001f) const;

	bool           validate(bool allowFail=false) const;
	bool           isNaN() const;

	bool           isYawOnly(void) const;
	bool           isTranslateOnly(void) const;

	void           setToScale(const Vector &scaleFactor);
	void           scale(float uniformScaleFactor);
	void           scalePosition_p(float uniformScaleFactor);

	const matrix_t &getMatrix() const { return matrix; }

private:

	bool           realValidate(bool allowFail) const;

private:

	matrix_t matrix;

};

// ======================================================================
// Turn this Transform into the identity transform 
//
// Remarks:
//
//   The identity Transform consists of a matrix of all 0 values with the exception of
//   the diagonal, which is all 1 values.  This matrix will result in no change when
//   multiplied against other matrices or transforming vectors.

inline void Transform::resetRotateTranslate_l2p(void)
{
	// make the diagonal 1's to form the identity matrix
	matrix[0][0] = CONST_REAL(1);
	matrix[0][1] = CONST_REAL(0);
	matrix[0][2] = CONST_REAL(0);
	matrix[0][3] = CONST_REAL(0);

	matrix[1][0] = CONST_REAL(0);
	matrix[1][1] = CONST_REAL(1);
	matrix[1][2] = CONST_REAL(0);
	matrix[1][3] = CONST_REAL(0);

	matrix[2][0] = CONST_REAL(0);
	matrix[2][1] = CONST_REAL(0);
	matrix[2][2] = CONST_REAL(1);
	matrix[2][3] = CONST_REAL(0);
}

// ----------------------------------------------------------------------
/**
 * Set the orientation to the identity orientation.
 * 
 * This routine does NOT affect position
 */

inline void Transform::resetRotate_l2p(void)
{
	matrix[0][0] = CONST_REAL(1);
	matrix[0][1] = CONST_REAL(0);
	matrix[0][2] = CONST_REAL(0);

	matrix[1][0] = CONST_REAL(0);
	matrix[1][1] = CONST_REAL(1);
	matrix[1][2] = CONST_REAL(0);

	matrix[2][0] = CONST_REAL(0);
	matrix[2][1] = CONST_REAL(0);
	matrix[2][2] = CONST_REAL(1);
}

// ----------------------------------------------------------------------
/**
 * Construct a Transform.
 * 
 * The transform will be initialized to the identity matrix.
 * 
 * @see Transform::makeIdentity()
 */

inline Transform::Transform(void)
{
	resetRotateTranslate_l2p();
}

// ----------------------------------------------------------------------

/**
 * Construct a Transform without initialization.
 * 
 * The transform matrix will not be initialized.  This 
 * member is intended for use only when efficiency is
 * a premium.
 * 
 * @see Transform::Transform()
 */

inline Transform::Transform(InitializeFlag noInitialization)
{
	UNREF(noInitialization);
} //lint !e1401 // warning, member 'matrix' not initialized // that's right, caller beware

// ----------------------------------------------------------------------
/**
 * Move the transform in it's parent space.
 * 
 * This routine moves the transform in it's parent space, or the world space if
 * the transform has no parent. Therefore, moving along the Z axis will move the
 * transform forward along the Z-axis of it's parent space, not forward in the
 * direction in which it is pointed.
 * 
 * @param vec  Displacement to move in parent space
 * @see Transform::move_l()
 */

inline void Transform::move_p(const Vector &vec)
{
	matrix[0][3] += vec.x;
	matrix[1][3] += vec.y;
	matrix[2][3] += vec.z;
}

// ----------------------------------------------------------------------
/**
 * Get the positional offset of this transform.
 * 
 * This routine returns a temporary.
 * 
 * The position returned is in parent space, which is world space if the
 * Transform has no parent.
 * 
 * @return The positional offest of this transform in parent space.
 */

inline const Vector Transform::getPosition_p(void) const
{
	return Vector(matrix[0][3], matrix[1][3], matrix[2][3]);
}

// ----------------------------------------------------------------------
/**
 * Set the positional offset of this transform.
 * 
 * The position is specified in parent space, which is world space if the
 * Transform has no parent.
 * 
 * @param vec  New translation for this transform
 */

inline void Transform::setPosition_p(const Vector &vec)
{
	matrix[0][3] = vec.x;
	matrix[1][3] = vec.y;
	matrix[2][3] = vec.z;
}

// ----------------------------------------------------------------------
/**
 * Set the positional offset of this transform.
 * 
 * The position is specified in parent space, which is world space if the
 * Transform has no parent.
 * 
 * @param x  New X translation for this transform
 * @param y  New Y translation for this transform
 * @param z  New Z translation for this transform
 */

inline void Transform::setPosition_p(real x, real y, real z)
{
	matrix[0][3] = x;
	matrix[1][3] = y;
	matrix[2][3] = z;
}

// ----------------------------------------------------------------------
/**
 * Set the transform matrix from the I, J, and K vectors.
 * 
 * This routine assumes that I, J, and K are a left-handed orthonormal basis.
 * If they are not, the reorthonormalize() routine must be called after this routine.
 * 
 * @param i  Unit vector along the X axis
 * @param j  Unit vector along the Y axis
 * @param k  Unit vector along the Z axis
 */

inline void Transform::setLocalFrameIJK_p(const Vector &i, const Vector &j, const Vector &k)
{
	matrix[0][0] = i.x;
	matrix[1][0] = i.y;
	matrix[2][0] = i.z;

	matrix[0][1] = j.x;
	matrix[1][1] = j.y;
	matrix[2][1] = j.z;

	matrix[0][2] = k.x;
	matrix[1][2] = k.y;
	matrix[2][2] = k.z;
}

// ----------------------------------------------------------------------
/**
 * Set the transform matrix from K and J vectors.
 * 
 * This routine assumes that K and J are part of a left-handed orthonormal basis.
 * If they are not, the reorthonormalize() routine must be called after this routine.
 * 
 * @param k  Unit vector along the Z axis
 * @param j  Unit vector along the Y axis
 */

inline void Transform::setLocalFrameKJ_p(const Vector &k, const Vector &j)
{
	Vector i = j.cross(k);
	setLocalFrameIJK_p(i, j, k);
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the X axis of this frame of reference.
 * 
 * This routine returns a temporary.
 * 
 * The vector returned is in parent space, which is world space if the
 * Transform has no parent.
 * 
 * @return The vector pointing along the X axis of the frame in parent space
 */

inline const Vector Transform::getLocalFrameI_p(void) const
{
	return Vector(matrix[0][0], matrix[1][0], matrix[2][0]);
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the Y axis of this frame of reference.
 * 
 * This routine returns a temporary.
 * 
 * The vector returned is in parent space, which is world space if the
 * Transform has no parent.
 * 
 * @return The vector pointing along the Y axis of the frame in parent space
 */

inline const Vector Transform::getLocalFrameJ_p(void) const
{
	return Vector(matrix[0][1], matrix[1][1], matrix[2][1]);
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the Z axis of this frame of reference.
 * 
 * This routine returns a temporary.
 * 
 * The position returned is in parent space, which is world space if the
 * Transform has no parent.
 * 
 * @return The vector pointing along the Z axis of the frame in parent space
 */

inline const Vector Transform::getLocalFrameK_p(void) const
{
	return Vector(matrix[0][2], matrix[1][2], matrix[2][2]);
}

// ----------------------------------------------------------------------
/**
 * Get the transform-space vector pointing along the X axis of the parent of reference.
 * 
 * This routine returns a temporary.
 * 
 * The vector returned is in local space.
 * 
 * @return The vector pointing along the X axis of the parent's frame in local space
 */

inline const Vector Transform::getParentFrameI_l(void) const
{
	return Vector(matrix[0][0], matrix[0][1], matrix[0][2]);
}

// ----------------------------------------------------------------------
/**
 * Get the transform-space vector pointing along the Y axis of the parent of reference.
 * 
 * This routine returns a temporary.
 * 
 * The vector returned is in local space.
 * 
 * @return The vector pointing along the Y axis of the parent's frame in local space
 */

inline const Vector Transform::getParentFrameJ_l(void) const
{
	return Vector(matrix[1][0], matrix[1][1], matrix[1][2]);
}

// ----------------------------------------------------------------------
/**
 * Get the transform-space vector pointing along the Z axis of the parent of reference.
 * 
 * This routine returns a temporary.
 * 
 * The vector returned is in local space.
 * 
 * @return The vector pointing along the Z axis of the parent's frame in local space
 */

inline const Vector Transform::getParentFrameK_l(void) const
{
	return Vector(matrix[2][0], matrix[2][1], matrix[2][2]);
}

// ----------------------------------------------------------------------
/**
 * Rotate vector from the matrix's current frame to the parent frame.
 * 
 * This routine returns a temporary.
 * 
 * Pure rotation is most useful for vectors that are orientational, such as
 * normals.
 * 
 * @param vec  Vector to rotate
 * @return The vector in parent space
 * @see Transform::rotateTranslate_l2p(), Transform::rotate_p2l()
 */

inline const Vector Transform::rotate_l2p(const Vector &vec) const
{
	return Vector(
		matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z,
		matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z,
		matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z);
}

// ----------------------------------------------------------------------
/**
 * Transform the vector from the matrix's current frame to the parent frame.
 * 
 * This routine returns a temporary.
 * 
 * Rotation and translation is most useful for vectors that are position, such as
 * vertex data.
 * 
 * @param vec  Vector to rotate and translate
 * @return The vector in parent space
 * @see Transform::rotate_l2p(), Transform::rotateTranslate_p2l()
 */

inline const Vector Transform::rotateTranslate_l2p(const Vector &vec) const
{
	return Vector (
		matrix[0][0] * vec.x + matrix[0][1] * vec.y + matrix[0][2] * vec.z + matrix[0][3],
		matrix[1][0] * vec.x + matrix[1][1] * vec.y + matrix[1][2] * vec.z + matrix[1][3],
		matrix[2][0] * vec.x + matrix[2][1] * vec.y + matrix[2][2] * vec.z + matrix[2][3]);
}

// ----------------------------------------------------------------------
/**
 * Rotate vector from the parent's space to the local transform space.
 * 
 * This routine returns a temporary.
 * 
 * Pure rotation is most useful for vectors that are orientational, such as
 * normals.
 * 
 * @param vec  Vector to rotate
 * @return The vector in local space
 * @see Transform::rotateTranslate_p2l(), Transform::rotate_l2p()
 */

inline const Vector Transform::rotate_p2l(const Vector &vec) const
{
	return Vector (
		matrix[0][0] * vec.x + matrix[1][0] * vec.y + matrix[2][0] * vec.z,
		matrix[0][1] * vec.x + matrix[1][1] * vec.y + matrix[2][1] * vec.z,
		matrix[0][2] * vec.x + matrix[1][2] * vec.y + matrix[2][2] * vec.z);
}

// ----------------------------------------------------------------------
/**
 * Transform the vector from the parent spave to the local transform space.
 * 
 * This routine returns a temporary.
 * 
 * Rotation and translation is most useful for vectors that are position, such as
 * vertex data.
 * 
 * @return The vector in local space
 * @see Transform::rotate_p2l(), Transform::rotateTranslate_l2p()
 */

inline const Vector Transform::rotateTranslate_p2l(const Vector &vec) const
{
	const real x = vec.x - matrix[0][3];
	const real y = vec.y - matrix[1][3];
	const real z = vec.z - matrix[2][3];

	return Vector(
		matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z,
		matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z,
		matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z);
}

// ----------------------------------------------------------------------
/**
 * Move the transform in it's own local space.
 * 
 * This routine moves the transform according to its current frame of reference.
 * Therefore, moving along the Z axis will move the transform forward in the direction
 * in which it is pointed.
 * 
 * @param vec  Vector to rotate and translate
 * @see Transform::moveInParentSpace()
 */

inline void Transform::move_l(const Vector &vec)
{
	move_p(rotate_l2p(vec));
}

// ----------------------------------------------------------------------
// Returns true if the rotation component of a transform is yaw-only 
// (no X- or Z-axis rotation)

// This test is very simple when we're guaranteed that our axes are orthonormal

inline bool Transform::isYawOnly(void) const
{
	return std::abs(matrix[1][1] - 1.0f) < 0.00001f;
}

// ----------------------------------------------------------------------
// Returns true if the transform does no rotation.

// This test is very simple when we're guaranteed that our axes are orthonormal

inline bool Transform::isTranslateOnly(void) const
{
	if(std::abs(matrix[0][0] - 1.0f) > 0.00001f) return false;
	if(std::abs(matrix[1][1] - 1.0f) > 0.00001f) return false;
	if(std::abs(matrix[2][2] - 1.0f) > 0.00001f) return false;

	return true;
}

// ----------------------------------------------------------------------

inline bool Transform::validate(bool allowFail) const
{
	UNREF(allowFail);
	// return realValidate(allowFail);
	return true;
}

// ----------------------------------------------------------------------
/**
 * Set the uniform scale factor applied to the transform's rotation
 * matrix.
 *
 * This function does not manipulate the position information within
 * the parent space.
 *
 * @param uniformScaleFactor  the scale factor to apply to the diagonal
 *                            of the rotation matrix.
 *
 * @see Transform::scalePosition_p().
 */

inline void Transform::scale(float uniformScaleFactor)
{
	matrix[0][0] *= uniformScaleFactor;
	matrix[1][1] *= uniformScaleFactor;
	matrix[2][2] *= uniformScaleFactor;
}

// ----------------------------------------------------------------------

/**
 * Scale the parent-space positioning information for the Transform.
 *
 * @param uniformScaleFactor  the scale factor to apply to the position
 *                            of the Transform in parent space.
 */

inline void Transform::scalePosition_p(float uniformScaleFactor)
{
	matrix[0][3] *= uniformScaleFactor;
	matrix[1][3] *= uniformScaleFactor;
	matrix[2][3] *= uniformScaleFactor;
}

// ----------------------------------------------------------------------

inline void Transform::ypr_l(Vector const & ypr)
{
	ypr_l(ypr.x, ypr.y, ypr.z);
}

// ======================================================================

#endif
