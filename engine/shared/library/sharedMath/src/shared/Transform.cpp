// ======================================================================
//
// Transform.cpp
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// matrix multiply
//
// a b c d     m n o p      am+bq+cu an+br+cv ao+bs+cw ap+bt+cx+d
// e f g h     q r s t      em+fq+gu en+fr+gv eo+fs+gw ep+ft+gx+h
// i j k l  *  u v w x  =   im+jq+ku in+jr+kv io+js+kw ip+jt+kx+l
// 0 0 0 1     0 0 0 1             0        0        0          1
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Transform.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/Quaternion.h"

#undef TRY_FOR_SSE
#define TRY_FOR_SSE WIN32

#if TRY_FOR_SSE
#include "sharedMath/SseMath.h"
#endif

#include <cfloat>

// ======================================================================

static void xf_matrix_3x4(float *out, const float *left, const float *right);
#if TRY_FOR_SSE
static void sse_xf_matrix_3x4(float *out, const float *left, const float *right);
#endif
// ======================================================================

namespace TransformNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;

	typedef void (*pf_multiply)(float *out, const float *left, const float *right);

	static pf_multiply s_mult_func;
}

using namespace TransformNamespace;

// ======================================================================

const Transform Transform::identity;

// ======================================================================

void TransformNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("Transform not installed."));
	s_installed = false;
}

// ======================================================================

void Transform::install()
{
	DEBUG_FATAL(s_installed, ("Transform already installed."));
#if TRY_FOR_SSE
	if (SseMath::canDoSseMath())
	{
		s_mult_func = sse_xf_matrix_3x4;
	}
	else
	{
		s_mult_func = xf_matrix_3x4;
	}
#else
	s_mult_func = xf_matrix_3x4;
#endif

	s_installed = true;
	ExitChain::add(remove, "Transform");
}

// ======================================================================
// Yaw this transform
//
// Remarks:
//
//   This routine will rotate the transform around the Y axis by the specified
//   number of radians.
//
//   Positive rotations are clockwise when viewed looking at the origin from
//   the positive side of the axis around which the transform is being rotated.
//
// See Also:
//
//   Transform::pitch_l(), Transform::roll_l(),

void Transform::yaw_l(
	real radians  // Amount to rotate, in radians
	)
{
	// a  b  c  d    C  0  S  0   aC-cS b aS+cC d
	// e  f  g  h *  0  1  0  0 = eC-gS f es+gC h
	// i  j  k  l   -S  0  C  0   iC-kS j iS+kS l
	// 0  0  0  1    0  0  0  1   0     0     0 1

	const real sine   = sin(radians);
	const real cosine = cos(radians);

	const real a = matrix[0][0];
	const real c = matrix[0][2];
	const real e = matrix[1][0];
	const real g = matrix[1][2];
	const real i = matrix[2][0];
	const real k = matrix[2][2];

	matrix[0][0] = a * cosine + c *  -sine;
	matrix[0][2] = a *   sine + c * cosine;

	matrix[1][0] = e * cosine + g *  -sine;
	matrix[1][2] = e *   sine + g * cosine;

	matrix[2][0] = i * cosine + k *  -sine;
	matrix[2][2] = i *   sine + k * cosine;
}

// ----------------------------------------------------------------------
/**
 * Pitch this transform.
 *
 * This routine will rotate the transform around the X axis by the specified
 * number of radians.
 *
 * Positive rotations are clockwise when viewed looking at the origin from
 * the positive side of the axis around which the transform is being rotated.
 *
 * @param radians  Amount to rotate, in radians
 * @see Transform::yaw_l(), Transform::roll_l(),
 */

void Transform::pitch_l(real radians)
{
	//  a  b  c  d   1  0  0  0    a bC+cS -bS+cC d
	//  e  f  g  h * 0  C -S  0 =  e fC+gS -fS+gC h
	//  i  j  k  l   0  S  C  0    i jC+kS -jS+kC l
	//  0  0  0  1   0  0  0  1    0     0      0 1

	const real sine   = sin(radians);
	const real cosine = cos(radians);

	const real b = matrix[0][1];
	const real c = matrix[0][2];
	const real f = matrix[1][1];
	const real g = matrix[1][2];
	const real j = matrix[2][1];
	const real k = matrix[2][2];

	matrix[0][1] = b * cosine + c *   sine;
	matrix[0][2] = b *  -sine + c * cosine;

	matrix[1][1] = f * cosine + g *   sine;
	matrix[1][2] = f *  -sine + g * cosine;

	matrix[2][1] = j * cosine + k *   sine;
	matrix[2][2] = j *  -sine + k * cosine;
}

// ----------------------------------------------------------------------
/**
 * Roll this transform.
 *
 * This routine will rotate the transform around the Z axis by the specified
 * number of radians.
 *
 * Positive rotations are clockwise when viewed looking at the origin from
 * the positive side of the axis around which the transform is being rotated.
 *
 * @param radians  Amount to rotate, in radians
 * @see Transform::yaw_l(), Transform::pitch_l(),
 */

void Transform::roll_l(real radians)
{
	// a  b  c  d   C -S  0  0   aC+bS -aS+bC c d
	// e  f  g  h * S  C  0  0 = eC+fS -eS+fC g h
	// i  j  k  l   0  0  1  0   iC+jS -iS+jC k l
	// 0  0  0  1   0  0  0  1       0      0 0 1

	const real sine   = sin(radians);
	const real cosine = cos(radians);

	const real a = matrix[0][0];
	const real b = matrix[0][1];
	const real e = matrix[1][0];
	const real f = matrix[1][1];
	const real i = matrix[2][0];
	const real j = matrix[2][1];

	matrix[0][0] = a * cosine + b *   sine;
	matrix[0][1] = a *  -sine + b * cosine;

	matrix[1][0] = e * cosine + f *   sine;
	matrix[1][1] = e *  -sine + f * cosine;

	matrix[2][0] = i * cosine + j *   sine;
	matrix[2][1] = i *  -sine + j * cosine;
}

// ----------------------------------------------------------------------

static void xf_matrix_3x4(float *out, const float *left, const float *right)
{
	if (left==out || right==out)
	{
		float temp[12];

		temp[0] = left[0] * right[0] + left[1] * right[4] + left[2] * right[8];
		temp[1] = left[0] * right[1] + left[1] * right[5] + left[2] * right[9];
		temp[2] = left[0] * right[2] + left[1] * right[6] + left[2] * right[10];
		temp[3] = left[0] * right[3] + left[1] * right[7] + left[2] * right[11] + left[3];

		temp[4] = left[4] * right[0] + left[5] * right[4] + left[6] * right[8];
		temp[5] = left[4] * right[1] + left[5] * right[5] + left[6] * right[9];
		temp[6] = left[4] * right[2] + left[5] * right[6] + left[6] * right[10];
		temp[7] = left[4] * right[3] + left[5] * right[7] + left[6] * right[11] + left[7];

		temp[8] = left[8] * right[0] + left[9] * right[4] + left[10] * right[8];
		temp[9] = left[8] * right[1] + left[9] * right[5] + left[10] * right[9];
		temp[10]= left[8] * right[2] + left[9] * right[6] + left[10] * right[10];
		temp[11]= left[8] * right[3] + left[9] * right[7] + left[10] * right[11] + left[11];

		out[0]=temp[0];
		out[1]=temp[1];
		out[2]=temp[2];
		out[3]=temp[3];
		out[4]=temp[4];
		out[5]=temp[5];
		out[6]=temp[6];
		out[7]=temp[7];
		out[8]=temp[8];
		out[9]=temp[9];
		out[10]=temp[10];
		out[11]=temp[11];
	}
	else
	{
		out[0] = left[0] * right[0] + left[1] * right[4] + left[2] * right[8];
		out[1] = left[0] * right[1] + left[1] * right[5] + left[2] * right[9];
		out[2] = left[0] * right[2] + left[1] * right[6] + left[2] * right[10];
		out[3] = left[0] * right[3] + left[1] * right[7] + left[2] * right[11] + left[3];

		out[4] = left[4] * right[0] + left[5] * right[4] + left[6] * right[8];
		out[5] = left[4] * right[1] + left[5] * right[5] + left[6] * right[9];
		out[6] = left[4] * right[2] + left[5] * right[6] + left[6] * right[10];
		out[7] = left[4] * right[3] + left[5] * right[7] + left[6] * right[11] + left[7];

		out[8] = left[8] * right[0] + left[9] * right[4] + left[10] * right[8];
		out[9] = left[8] * right[1] + left[9] * right[5] + left[10] * right[9];
		out[10]= left[8] * right[2] + left[9] * right[6] + left[10] * right[10];
		out[11]= left[8] * right[3] + left[9] * right[7] + left[10] * right[11] + left[11];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if TRY_FOR_SSE
__declspec(naked) void sse_xf_matrix_3x4(float *out, const float *left, const float *right)
{
	UNREF(left);
	UNREF(right);
	UNREF(out);
	__asm {
		push eax
		push ebx
		add  esp, -48

		// [esp + 60] == out
		// [esp + 64] == left
		// [esp + 68] == right
		mov  eax, [esp + 64] // left
		mov  ebx, [esp + 68] // right

		movups	xmm0, [ebx +  0]	// right: row0
		movups	xmm1, [ebx + 16]	// right: row1
		movups	xmm2, [ebx + 32]	// right: row2

		// 16-byte align data pointer
		lea	ebx, [esp+15]
		and	ebx, -16

		///////////////////////////////////////////////////
		movaps	xmm3, xmm0			// right: row0
		movss		xmm4, [eax + 0]   // left[0][0]
		shufps	xmm4, xmm4, 0
		mulps		xmm3, xmm4

		movaps	xmm5, xmm1			// right: row1
		movss		xmm6, [eax + 4]	// left[0][1]
		shufps	xmm6, xmm6, 0
		mulps		xmm5, xmm6

		addps		xmm5, xmm3

		movaps	xmm3, xmm2			// right: row2
		movss		xmm4, [eax + 8]   // left[0][2]
		shufps	xmm4, xmm4, 0
		mulps		xmm3, xmm4

		addps		xmm5, xmm3

		movss		xmm7, [eax + 12] // left[0][3]
		shufps   xmm7, xmm7,  0x15 //0001 0101
		addps		xmm5, xmm7

		movaps	[ebx], xmm5
		///////////////////////////////////////////////////
		movaps	xmm3, xmm0			// right: row0
		movss		xmm4, [eax + 16]  // left[1][0]
		shufps	xmm4, xmm4, 0
		mulps		xmm3, xmm4

		movaps	xmm5, xmm1			// right: row1
		movss		xmm6, [eax + 20]	// left[1][1]
		shufps	xmm6, xmm6, 0
		mulps		xmm5, xmm6

		addps		xmm5, xmm3

		movaps	xmm3, xmm2			// right: row2
		movss		xmm4, [eax + 24]   // left[1][2]
		shufps	xmm4, xmm4, 0
		mulps		xmm3, xmm4

		addps		xmm5, xmm3

		movss		xmm7, [eax + 28] // left[1][3]
		shufps   xmm7, xmm7,  0x15 //0001 0101
		addps		xmm5, xmm7

		movaps	[ebx+16], xmm5
		///////////////////////////////////////////////////
		movaps	xmm3, xmm0			// right: row0
		movss		xmm4, [eax + 32]  // left[2][0]
		shufps	xmm4, xmm4, 0
		mulps		xmm3, xmm4

		movaps	xmm5, xmm1			// right: row1
		movss		xmm6, [eax + 36]	// left[2][1]
		shufps	xmm6, xmm6, 0
		mulps		xmm5, xmm6

		addps		xmm5, xmm3

		movaps	xmm3, xmm2			// right: row2
		movss		xmm4, [eax + 40]   // left[2][2]
		shufps	xmm4, xmm4, 0
		mulps		xmm3, xmm4

		addps		xmm5, xmm3

		movss		xmm7, [eax + 44] // left[2][3]
		shufps   xmm7, xmm7,  0x15 //0001 0101
		addps		xmm5, xmm7

		mov		eax, [esp+60]

		movups	[eax+32], xmm5
		///////////////////////////////////////////////////

		movaps   xmm1, [ebx+ 0]
		movaps   xmm2, [ebx+16]
		movups   [eax+0 ], xmm1
		movups   [eax+16], xmm2

		mov	ebx, [esp + 48]
		mov	eax, [esp + 52]
		add   esp, 56
		ret
	}
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
/**
 * Multiply two matrices together.
 *
 * This routine will properly handle the case where the destination matrix
 * is also one or both of the source matrices.
 *
 * @param lhs  Transform on the left-hand side
 * @param rhs  Transform on the right-hand side
 */

void Transform::multiply(const Transform &lhs, const Transform &rhs)
{
	s_mult_func(matrix[0], lhs.matrix[0], rhs.matrix[0]);
}

// ----------------------------------------------------------------------
/**
 * Invert a simple transform.
 *
 * The source matrix has to be composed of purely rotational and translational
 * components.  It may not contain any scaling or shearing transforms.
 *
 * @param transform  Simple transform to invert
 */

void Transform::invert(const Transform &transform)
{
	// transpose the upper 3x3 matrix
	matrix[0][0] = transform.matrix[0][0];
	matrix[0][1] = transform.matrix[1][0];
	matrix[0][2] = transform.matrix[2][0];

	matrix[1][0] = transform.matrix[0][1];
	matrix[1][1] = transform.matrix[1][1];
	matrix[1][2] = transform.matrix[2][1];

	matrix[2][0] = transform.matrix[0][2];
	matrix[2][1] = transform.matrix[1][2];
	matrix[2][2] = transform.matrix[2][2];

	// invert the translation
	const real x = transform.matrix[0][3];
	const real y = transform.matrix[1][3];
	const real z = transform.matrix[2][3];
	matrix[0][3] = -(matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z);
	matrix[1][3] = -(matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z);
	matrix[2][3] = -(matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z);
}

// ----------------------------------------------------------------------
/**
 * Reorthogonalize a transform.
 *
 * Repeated rotations will introduce numerical error into the transform,
 * which will cause the upper 3x3 matrix to become non-orthonormal.  If
 * enough error is introduced, weird errors will begin to occur when using
 * the transform.
 *
 * This routine attempts to reduce the numerical error by reorthonormalizing
 * the upper 3x3 matrix.
 */

void Transform::reorthonormalize(void)
{
	Vector k = getLocalFrameK_p();
	Vector j = getLocalFrameJ_p();

	if (!k.normalize())
	{
		DEBUG_FATAL(true, ("could not normalize frame k"));
		k = Vector::unitZ; //lint !e527 // Unreachable
	}

	if (!j.normalize())
	{
		DEBUG_FATAL(true, ("could not normalize frame j"));
		j = Vector::unitY; //lint !e527 // Unreachable
	}

	// build the remaining vector with the cross product
	Vector i = j.cross(k);

	// use that result to rebuild the
	j = k.cross(i);

	// copy the results back into the transform
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
 * Send this transform to the DebugPrint system.
 *
 * The header parameter may be nullptr.
 *
 * @param header  Header for the transform
 */

void Transform::debugPrint(const char *header) const
{
	if (header)
		DEBUG_REPORT_PRINT(true, ("%s\n", header));

	for (int i = 0; i < 3; ++i)
		DEBUG_REPORT_PRINT(true, ("  %-8.2f %-8.2f %-8.2f %-8.2f\n", matrix[i][0], matrix[i][1], matrix[i][2], matrix[i][3]));
}

// ----------------------------------------------------------------------
/**
 * Rotate an array of vector from the matrix's current frame to the parent frame.
 *
 * Pure rotation is most useful for vectors that are orientational, such as
 * normals.
 *
 * The source and result arrays may be the same array.
 *
 * @param source  Source array of vectors to transform
 * @param result  Array to store the result of the transforms
 * @param count  Number of vertices to transform
 * @see Transform::rotateTranslate_l2p(), Transform::rotate_p2l()
 */

void Transform::rotate_l2p(const Vector *source, Vector *result, int count) const
{
	DEBUG_FATAL(!source, ("source array is nullptr"));
	DEBUG_FATAL(!result, ("result array is nullptr"));
	DEBUG_FATAL(source == result, ("source and result array can not be the same"));

    NOT_NULL(source);
    NOT_NULL(result);

	for ( ; count; --count, ++source, ++result)
	{
		const real x = source->x;
		const real y = source->y;
		const real z = source->z;
		result->x = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
		result->y = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
		result->z = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;
	}
}

// ----------------------------------------------------------------------
/**
 * Transform an array of vectors from the matrix's current frame to the parent frame.
 *
 * Rotation and translation is most useful for vectors that are position, such as
 * vertex data.
 *
 * The source and result arrays may be the same array.
 *
 * @param source  Source array of vectors to transform
 * @param result  Array to store the result of the transforms
 * @param count  Number of vertices to transform
 * @see Transform::rotate_l2p(), Transform::rotateTranslate_p2l()
 */

void Transform::rotateTranslate_l2p(const Vector *source, Vector *result, int count) const
{
	DEBUG_FATAL(!source, ("source array is nullptr"));
	DEBUG_FATAL(!result, ("result array is nullptr"));

    NOT_NULL(source);
    NOT_NULL(result);

	for ( ; count; --count, ++source, ++result)
	{
		const real x = source->x;
		const real y = source->y;
		const real z = source->z;

		result->x = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z + matrix[0][3];
		result->y = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z + matrix[1][3];
		result->z = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z + matrix[2][3];
	}
}

// ----------------------------------------------------------------------
/**
 * Rotate an array of vectors from the parent's space to the local transform space.
 *
 * Pure rotation is most useful for vectors that are orientational, such as
 * normals.
 *
 * The source and result arrays may be the same array.
 *
 * @param source  Source array of vectors to transform
 * @param result  Array to store the result of the transforms
 * @param count  Number of vertices to transform
 * @see Transform::rotateTranslate_p2l(), Transform::rotate_l2p()
 */

void Transform::rotate_p2l(const Vector *source, Vector *result, int count) const
{
	DEBUG_FATAL(!source, ("source array is nullptr"));
	DEBUG_FATAL(!result, ("result array is nullptr"));

    NOT_NULL(source);
    NOT_NULL(result);

	for ( ; count; --count, ++source, ++result)
	{
		const real x = source->x;
		const real y = source->y;
		const real z = source->z;

		result->x = matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z;
		result->y = matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z;
		result->z = matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z;
	}
}

// ----------------------------------------------------------------------
/**
 * Transform an array of vectors from the parent spave to the local transform space.
 *
 * Rotation and translation is most useful for vectors that are position, such as
 * vertex data.
 *
 * The source and result arrays may be the same array.
 *
 * @param source  Source array of vectors to transform
 * @param result  Array to store the result of the transforms
 * @param count  Number of vertices to transform
 * @see Transform::rotate_p2l(), Transform::rotateTranslate_l2p()
 */

void Transform::rotateTranslate_p2l(const Vector *source, Vector *result, int count) const
{
	DEBUG_FATAL(!source, ("source array is nullptr"));
	DEBUG_FATAL(!result, ("result array is nullptr"));

    NOT_NULL(source);
    NOT_NULL(result);

	for ( ; count; --count, ++source, ++result)
	{
		const real x = source->x - matrix[0][3];
		const real y = source->y - matrix[1][3];
		const real z = source->z - matrix[2][3];

		result->x = matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z;
		result->y = matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z;
		result->z = matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z;
	}
}

// ----------------------------------------------------------------------

const Transform Transform::rotateTranslate_l2p(const Transform &t) const
{
	const Vector i = rotate_l2p(t.getLocalFrameI_p());
	const Vector j = rotate_l2p(t.getLocalFrameJ_p());
	const Vector k = rotate_l2p(t.getLocalFrameK_p());
	const Vector p = rotateTranslate_l2p(t.getPosition_p());

	Transform tmp;
	tmp.setLocalFrameIJK_p(i, j, k);
	tmp.setPosition_p(p);
	tmp.reorthonormalize();

	return tmp;
}

// ----------------------------------------------------------------------

const Transform Transform::rotateTranslate_p2l(const Transform &t) const
{
	const Vector i = rotate_p2l(t.getLocalFrameI_p());
	const Vector j = rotate_p2l(t.getLocalFrameJ_p());
	const Vector k = rotate_p2l(t.getLocalFrameK_p());
	const Vector p = rotateTranslate_p2l(t.getPosition_p());

	Transform tmp;
	tmp.setLocalFrameIJK_p(i, j, k);
	tmp.setPosition_p(p);
	tmp.reorthonormalize();

	return tmp;
}

// ----------------------------------------------------------------------

bool Transform::operator== (const Transform& rhs) const
{
	return (memcmp (&rhs, this, sizeof(Transform) ) == 0);
}

// ----------------------------------------------------------------------

bool Transform::operator!= (const Transform& rhs) const
{
	return !operator== (rhs);
}

// ----------------------------------------------------------------------

bool Transform::approximates(const Transform &rhs, float rotDelta, float posDelta) const
{
	rotDelta = 1-rotDelta;
	posDelta *= posDelta;

	float dot[3];
	int i;
	for (i=0; i<3; ++i)
	{
		dot[i] = matrix[0][i]*rhs.matrix[0][i] + matrix[1][i]*rhs.matrix[1][i] + matrix[2][i]*rhs.matrix[2][i];
	}

	float dx = matrix[0][3]-rhs.matrix[0][3];
	float dy = matrix[1][3]-rhs.matrix[1][3];
	float dz = matrix[2][3]-rhs.matrix[2][3];
	
	for (i=0; i<3; ++i)
	{
		if (dot[i] < rotDelta)
			return false;
	}

	return dx*dx+dy*dy+dz*dz <= posDelta;
}

// ----------------------------------------------------------------------
/**
 * Set this Transform to a matrix that does nothing but scale
 * objects by the specified amount when computing from local to
 * parent space.
 *
 * @param scale  the scale factor applied to x, y, z when transforming from local to parent space.
 */

void Transform::setToScale(const Vector &scaleFactor)
{
	matrix[0][0] = scaleFactor.x;
	matrix[0][1] = 0.0f;
	matrix[0][2] = 0.0f;
	matrix[0][3] = 0.0f;

	matrix[1][0] = 0.0f;
	matrix[1][1] = scaleFactor.y;
	matrix[1][2] = 0.0f;
	matrix[1][3] = 0.0f;

	matrix[2][0] = 0.0f;
	matrix[2][1] = 0.0f;
	matrix[2][2] = scaleFactor.z;
	matrix[2][3] = 0.0f;
}

// ----------------------------------------------------------------------

void Transform::ypr_l(real const y, real const p, real const r)
{
	real const cy = cos(y);
	real const sy = sin(y);
	real const cp = cos(p);
	real const sp = sin(p);
	real const cr = cos(r);
	real const sr = sin(r);
	
	matrix[0][0] = sp * sr * sy + cr * cy;
	matrix[0][1] = cr * sp * sy - cy * sr;
	matrix[0][2] = cp * sy;

	matrix[1][0] = cp * sr;
	matrix[1][1] = cp * cr;
	matrix[1][2] = -sp;

	matrix[2][0] = cy * sp * sr - cr * sy;
	matrix[2][1] = cr * cy * sp + sr * sy;
	matrix[2][2] = cp * cy;
}

// ----------------------------------------------------------------------

namespace
{
	class ValidateBad
	{
	public:
		ValidateBad();
		~ValidateBad();
		void checked();
	private:
		int m_checked;
	};
}

ValidateBad::ValidateBad()
: m_checked(0)
{
}

ValidateBad::~ValidateBad()
{
	REPORT_LOG(m_checked != 0, ("%d transforms checked", m_checked));
}

void ValidateBad::checked()
{
	++m_checked;
}

static float magnitudeThreshold = 0.0001f;
static float dotThreshold = cos(convertDegreesToRadians(0.05f));

static bool validate(bool allowFail, Vector const & a, Vector const & b)
{
	const float magnitude = a.magnitude();
	if (!WithinEpsilonInclusive(1.0f, magnitude, magnitudeThreshold))
	{
		FATAL(!allowFail, ("Transform magnitude %0.12f", magnitude));
		return false;
	}

    Vector a2 = a;

	IGNORE_RETURN( a2.normalize() );

	const float dot = a2.dot(b);
	if (dot < dotThreshold)
	{
		FATAL(!allowFail, ("Transform angle off %0.12f", dot));
		return false;
	}

	return true;
}

bool Transform::isNaN() const
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
#ifdef WIN32
			if(_isnan( static_cast<double>(matrix[i][j])))
			{
				DEBUG_FATAL(true, ("nan"));
				return true;
			}
#else
			if(std::isnan(matrix[i][j]))
			{
				DEBUG_FATAL(true, ("nan"));
				return true;
			}
#endif
		}
	}
	return false;
}

bool Transform::realValidate(bool allowFail) const
{
	static ValidateBad vb;
	vb.checked();

	IGNORE_RETURN(isNaN());

	Transform t = *this;
	t.reorthonormalize();
	t.reorthonormalize();
	t.reorthonormalize();

	if (!::validate(allowFail,   getLocalFrameI_p(), t.getLocalFrameI_p()))
		return false;
	if (!::validate(allowFail,   getLocalFrameJ_p(), t.getLocalFrameJ_p()))
		return false;
	if (!::validate(allowFail,   getLocalFrameK_p(), t.getLocalFrameK_p()))
		return false;

	Quaternion q(*this);
	Transform u(IF_none);
	q.getTransform(&u);

	if (!::validate(allowFail, u.getLocalFrameI_p(), t.getLocalFrameI_p()))
		return false;
	if (!::validate(allowFail, u.getLocalFrameJ_p(), t.getLocalFrameJ_p()))
		return false;
	if (!::validate(allowFail, u.getLocalFrameK_p(), t.getLocalFrameK_p()))
		return false;

	return true;
}

// ======================================================================
