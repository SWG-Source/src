// ======================================================================
//
// Hsv.cpp
// copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Hsv.h"

#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMath/VectorRgba.h"

#include <algorithm>

// ======================================================================

void Hsv::rgbToHsv(float const red, float const green, float const blue, float &hue, float &saturation, float &value)
{
	float const min = std::min(red, std::min(green, blue));
	float const max = std::max(red, std::max(green, blue));
	float const delta = (max - min);

	hue = 0.0f;
	saturation = 0.0f;
	value = max;

	if (max > 0.0f)
	{
		saturation = delta / max;
	}
	else
	{
		hue = -1.0f;

		return;
	}

	if (delta > 0.0f)
	{
		if (WithinEpsilonInclusive(red, max, 0.0001f))
		{
			hue = (green - blue) / delta; // between yellow and magenta
		}
		else if (WithinEpsilonInclusive(green, max, 0.0001f))
		{
			hue = 2.0f + (blue - red) / delta; // between cyan and yellow
		}
		else
		{
			hue = 4.0f + (red - green) / delta; // between magenta and cyan
		}
	}

	hue *= 60.0f; // degrees

	if (hue < 0.0f)
	{
		hue += 360.0f;
	}
}

// ----------------------------------------------------------------------

void Hsv::rgbToHsv(VectorArgb const & rgb, Vector & hsv )
{
	rgbToHsv(rgb.r, rgb.g, rgb.b, hsv.x, hsv.y, hsv.z);
}

// ----------------------------------------------------------------------

void Hsv::rgbToHsv(VectorRgba const & rgb, Vector & hsv )
{
	rgbToHsv(rgb.r, rgb.g, rgb.b, hsv.x, hsv.y, hsv.z);
}

//===================================================================


