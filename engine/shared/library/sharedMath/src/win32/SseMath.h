// ======================================================================
//
// SseMath.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SseMath_H
#define INCLUDED_SseMath_H

// ======================================================================

class Transform;
class Vector;

// ======================================================================
#define MXCSR_FLUSH_TO_ZERO        (1<<15)
#define MXCSR_PRECISION_MASK       (1<<12)
#define MXCSR_UNDERFLOW_MASK       (1<<11)
#define MXCSR_OVERFLOW_MASK        (1<<10)
#define MXCSR_DIVIDE_BY_ZERO_MASK  (1<< 9)
#define MXCSR_DENORMAL_MASK        (1<< 8)

class SseMath
{
public:

	static bool    canDoSseMath();

	static Vector  rotateTranslateScale_l2p(const Transform &transform, const Vector &vector, float scale);
	static Vector  rotateScale_l2p(const Transform &transform, const Vector &vector, float scale);

	static void    skinPositionNormal_l2p(const Transform &transform, const Vector &position, const Vector &normal, float scale, Vector &destPosition, Vector &destNormal);
	static void    skinPositionNormalAdd_l2p(const Transform &transform, const Vector &position, const Vector &normal, float scale, Vector &destPosition, Vector &destNormal);
	static void prefetch(void const * const sourceData, size_t const objectSize);
};

// ----------------------------------------------------------------------

inline void SseMath::prefetch(void const * const sourceData, size_t const objectSize)
{
#if defined(_MSC_VER)
	_asm
	{ 
		mov esi, sourceData
		prefetchnta objectSize[esi]
	}
#else
	// rls - add linux version here.
	UNREF(sourceData);
	UNREF(objectSize);
#endif
}

// ======================================================================

#endif
