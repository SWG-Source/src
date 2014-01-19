// ======================================================================
//
// FloatMath.h
// Portions Copyright 1998 Bootprint Entertainment
// Portions Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_FloatMath_H
#define INCLUDED_FloatMath_H

// ======================================================================

float const PI          = 3.14159265358979323846f;
float const PI_TIMES_2  = PI * 2.0f;
float const PI_OVER_2   = PI / 2.0f;
float const PI_OVER_3   = PI / 3.0f;
float const PI_OVER_4   = PI / 4.0f;
float const PI_OVER_6   = PI / 6.0f;
float const PI_OVER_8   = PI / 8.0f;
float const PI_OVER_12  = PI / 12.0f;
float const PI_OVER_16  = PI / 16.0f;
float const PI_OVER_180 = PI / 180.0f;

float const E           = 2.7182818284590452f;

// ======================================================================

#ifdef PLATFORM_WIN32

#if _MSC_VER < 1300

inline float abs(float f)
{
	return fabsf(f);
}

inline float sqrt(float f)
{
	return sqrtf(f);
}

inline float sin(float f)
{
	return sinf(f);
}

inline float cos(float f)
{
	return cosf(f);
}

inline float tan(float f)
{
	return tanf(f);
}

inline float asin(float f)
{
	return asinf(f);
}

inline float acos(float f)
{
	return acosf(f);
}

inline float atan2(float y, float x)
{
	return atan2f(y, x);
}

inline float pow(float x, float y)
{
	return powf(x, y);
}

#endif // _MSC_VER < 1300

#else

// @todo check standard, if cmath.h will not be deprecated,
// include cmath.h here and get rid of the redundant inlines.
#include <cmath>

#ifndef LINUX
// The Linux setup defines this in PlatformGlue.h
inline float abs(float f)
{
	return std::abs(f);
}

inline float sqrt(float f)
{
	return std::sqrt(f);
}

inline float sin(float f)
{
	return std::sin(f);
}

inline float cos(float f)
{
	return std::cos(f);
}

inline float tan(float f)
{
	return std::tan(f);
}

inline float asin(float f)
{
	return std::asin(f);
}

inline float acos(float f)
{
	return std::acos(f);
}

inline float atan2(float y, float x)
{
	return std::atan2(y, x);
}

inline float pow(float x, float y)
{
	return std::pow(x, y);
}
#endif

#endif

// ======================================================================

inline float convertDegreesToRadians (real degrees)
{
	return degrees * PI / 180.0f;
}

inline float convertRadiansToDegrees (real radians)
{
	return radians * 180.0f / PI;
}

inline float cot(float f)
{
	return RECIP(tanf(f));
}

inline float GaussianDistribution(float variate, float standardDeviation, float mean)
{
	// equation from http://mathworld.wolfram.com/NormalDistribution.html
	return (1.0f / (standardDeviation * sqrt(2.0f * PI))) * pow(E, -(sqr(variate - mean) / (2.0f * sqr(standardDeviation))));
}

inline float withinEpsilon(float const rhs, float const lhs, float const epsilon = 1.0e-3f)
{
	return ((lhs - epsilon) <= rhs) && (rhs <= (lhs + epsilon));
}

// ======================================================================

#endif
