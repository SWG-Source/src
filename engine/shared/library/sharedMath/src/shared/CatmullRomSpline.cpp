// ============================================================================
//
// CatmullRomSpline.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/CatmullRomSpline.h"

#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------------
void CatmullRomSpline::getCatmullRomSplinePoint(float const c1x, float const c1y, float const c2x, float const c2y, float const c3x, float const c3y, float const c4x, float const c4y, float const t, float &resultX, float &resultY)
{
	float const t3 = t * t * t;
	float const t2 = t * t;
	float const a = (-0.5f * t3 + t2 - 0.5f * t);
	float const b = (1.5f * t3 - 2.5f * t2 + 1.0f);
	float const c = (-1.5f * t3 + 2.0f * t2 + 0.5f * t);
	float const d = (0.5f * t3 - 0.5f * t2);

	resultX = c1x * a + c2x * b + c3x * c + c4x * d;
	resultY = c1y * a + c2y * b + c3y * c + c4y * d;
}

//-----------------------------------------------------------------------------
void CatmullRomSpline::getCatmullRomSplinePoint3d(Vector const &c1, Vector const &c2, Vector const &c3, Vector const &c4, float const t, Vector &result)
{
	float const t3 = t * t * t;
	float const t2 = t * t;
	float const a = (-0.5f * t3 + t2 - 0.5f * t);
	float const b = (1.5f * t3 - 2.5f * t2 + 1.0f);
	float const c = (-1.5f * t3 + 2.0f * t2 + 0.5f * t);
	float const d = (0.5f * t3 - 0.5f * t2);

	result.x = c1.x * a + c2.x * b + c3.x * c + c4.x * d;
	result.y = c1.y * a + c2.y * b + c3.y * c + c4.y * d;
	result.z = c1.z * a + c2.z * b + c3.z * c + c4.z * d;
}

// ============================================================================
