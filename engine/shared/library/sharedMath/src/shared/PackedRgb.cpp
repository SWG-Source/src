//===================================================================
//
// PackedRgb.cpp
// asommers 6-20-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PackedRgb.h"

#include "sharedMath/VectorArgb.h"

//===================================================================

const float     PackedRgb::oo255 = RECIP (255);

const PackedRgb PackedRgb::solidBlack   (  0,   0,   0);
const PackedRgb PackedRgb::solidBlue    (  0,   0, 255);
const PackedRgb PackedRgb::solidCyan    (  0, 255, 255);
const PackedRgb PackedRgb::solidGreen   (  0, 255,   0);
const PackedRgb PackedRgb::solidRed     (255,   0,   0);
const PackedRgb PackedRgb::solidMagenta (255,   0, 255);
const PackedRgb PackedRgb::solidYellow  (255, 255,   0);
const PackedRgb PackedRgb::solidWhite   (255, 255, 255);
const PackedRgb PackedRgb::solidGray    (128, 128, 128);
const PackedRgb PackedRgb::solidOrange  (255, 128, 0);

//===================================================================

VectorArgb PackedRgb::convert (float alpha) const
{
	return VectorArgb (
		alpha, 
		static_cast<float> (r) * oo255, 
		static_cast<float> (g) * oo255,
		static_cast<float> (b) * oo255);
}

//-------------------------------------------------------------------

void PackedRgb::convert (const VectorArgb& color)
{
	r = static_cast<uint8> (color.r * 255.f);
	g = static_cast<uint8> (color.g * 255.f);
	b = static_cast<uint8> (color.b * 255.f);
}

//-------------------------------------------------------------------

bool PackedRgb::operator== (const PackedRgb& rhs) const
{
	return r == rhs.r && g == rhs.g && b == rhs.b;
}

//-------------------------------------------------------------------

bool PackedRgb::operator!= (const PackedRgb& rhs) const
{
	return r != rhs.r || g != rhs.g || b != rhs.b;
}

//-------------------------------------------------------------------

const PackedRgb PackedRgb::linearInterpolate (const PackedRgb& color1, const PackedRgb& color2, float t)
{
	return PackedRgb (
		static_cast<uint8> (::linearInterpolate (static_cast<int> (color1.r), static_cast<int> (color2.r), t)),
		static_cast<uint8> (::linearInterpolate (static_cast<int> (color1.g), static_cast<int> (color2.g), t)),
		static_cast<uint8> (::linearInterpolate (static_cast<int> (color1.b), static_cast<int> (color2.b), t)));
}

//===================================================================

