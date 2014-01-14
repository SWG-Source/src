// ======================================================================
//
// PackedArgb.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PackedArgb.h"

#include "sharedMath/VectorArgb.h"

// ======================================================================

const real PackedArgb::oo255 = RECIP (255);

const PackedArgb PackedArgb::solidBlack   (255,   0,   0,   0);
const PackedArgb PackedArgb::solidBlue    (255,   0,   0, 255);
const PackedArgb PackedArgb::solidCyan    (255,   0, 255, 255);
const PackedArgb PackedArgb::solidGreen   (255,   0, 255,   0);
const PackedArgb PackedArgb::solidRed     (255, 255,   0,   0);
const PackedArgb PackedArgb::solidMagenta (255, 255,   0, 255);
const PackedArgb PackedArgb::solidYellow  (255, 255, 255,   0);
const PackedArgb PackedArgb::solidWhite   (255, 255, 255, 255);
const PackedArgb PackedArgb::solidGray    (255, 128, 128, 128);

// ======================================================================

PackedArgb const PackedArgb::linearInterpolate(PackedArgb const & color1, PackedArgb const & color2, float const t)
{
	return PackedArgb(
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getA()), static_cast<int>(color2.getA()), t)),
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getR()), static_cast<int>(color2.getR()), t)),
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getG()), static_cast<int>(color2.getG()), t)),
		static_cast<uint8>(::linearInterpolate(static_cast<int>(color1.getB()), static_cast<int>(color2.getB()), t)));
}

// ======================================================================

/**
 * Construct a PackedArgb value.
 * @param argb The initial component values.
 */

//#include "sharedMath/VectorArgb.h"

PackedArgb::PackedArgb(const VectorArgb &argb)
: m_argb(convert(argb.a, argb.r, argb.g, argb.b))
{
}

// ----------------------------------------------------------------------
/**
 * Set the color.
 * @argb The new alpha and color value.
 */

void PackedArgb::setArgb(const VectorArgb &argb)
{
	m_argb = convert(argb.a, argb.r, argb.g, argb.b);
}

// ======================================================================

