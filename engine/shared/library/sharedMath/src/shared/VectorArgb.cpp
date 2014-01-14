// ======================================================================
//
// VectorArgb.h
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/VectorArgb.h"

#include "sharedMath/PackedArgb.h"

// ======================================================================

const VectorArgb VectorArgb::solidBlack  (CONST_REAL(1), CONST_REAL(0),   CONST_REAL(0),   CONST_REAL(0));
const VectorArgb VectorArgb::solidBlue   (CONST_REAL(1), CONST_REAL(0),   CONST_REAL(0),   CONST_REAL(1));
const VectorArgb VectorArgb::solidCyan   (CONST_REAL(1), CONST_REAL(0),   CONST_REAL(1),   CONST_REAL(1));
const VectorArgb VectorArgb::solidGreen  (CONST_REAL(1), CONST_REAL(0),   CONST_REAL(1),   CONST_REAL(0));
const VectorArgb VectorArgb::solidRed    (CONST_REAL(1), CONST_REAL(1),   CONST_REAL(0),   CONST_REAL(0));
const VectorArgb VectorArgb::solidMagenta(CONST_REAL(1), CONST_REAL(1),   CONST_REAL(0),   CONST_REAL(1));
const VectorArgb VectorArgb::solidYellow (CONST_REAL(1), CONST_REAL(1),   CONST_REAL(1),   CONST_REAL(0));
const VectorArgb VectorArgb::solidWhite  (CONST_REAL(1), CONST_REAL(1),   CONST_REAL(1),   CONST_REAL(1));
const VectorArgb VectorArgb::solidGray   (CONST_REAL(1), CONST_REAL(0.5), CONST_REAL(0.5), CONST_REAL(0.5));

const real oo255 = 1.0f / 255.0f;

// ======================================================================

VectorArgb::VectorArgb(const PackedArgb &argb)
: a(static_cast<float>(argb.getA()) * oo255),
	r(static_cast<float>(argb.getR()) * oo255),
	g(static_cast<float>(argb.getG()) * oo255),
	b(static_cast<float>(argb.getB()) * oo255)
{
}

// ======================================================================
