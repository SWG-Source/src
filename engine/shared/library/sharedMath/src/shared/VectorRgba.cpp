// ======================================================================
//
// VectorRgba.h
//
// Portions Copyright 1999 Bootprint Entertainment
// Portions Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/VectorRgba.h"

#include "sharedMath/PackedArgb.h"

// ======================================================================

const VectorRgba VectorRgba::solidBlack  (0.0f, 0.0f, 0.0f, 1.0f);
const VectorRgba VectorRgba::solidBlue   (0.0f, 0.0f, 1.0f, 1.0f);
const VectorRgba VectorRgba::solidCyan   (0.0f, 1.0f, 1.0f, 1.0f);
const VectorRgba VectorRgba::solidGreen  (0.0f, 1.0f, 0.0f, 1.0f);
const VectorRgba VectorRgba::solidRed    (1.0f, 0.0f, 0.0f, 1.0f);
const VectorRgba VectorRgba::solidMagenta(1.0f, 0.0f, 1.0f, 1.0f);
const VectorRgba VectorRgba::solidYellow (1.0f, 1.0f, 0.0f, 1.0f);
const VectorRgba VectorRgba::solidWhite  (1.0f, 1.0f, 1.0f, 1.0f);
const VectorRgba VectorRgba::solidGray   (0.5f, 0.5f, 0.5f, 1.0f);

namespace VectorRgbaNamespace
{
	const real oo255 = 1.0f / 255.0f;
}
using namespace VectorRgbaNamespace;

// ======================================================================

VectorRgba::VectorRgba(PackedArgb const & argb)
:
	r(static_cast<float>(argb.getR()) * oo255),
	g(static_cast<float>(argb.getG()) * oo255),
	b(static_cast<float>(argb.getB()) * oo255),
	a(static_cast<float>(argb.getA()) * oo255)
{
}

// ======================================================================
