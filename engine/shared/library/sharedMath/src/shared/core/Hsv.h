//===================================================================
//
// Hsv.h
// copyright 2005 Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_Hsv_H
#define INCLUDED_Hsv_H

//===================================================================

class Vector;
class VectorArgb;
class VectorRgba;

//===================================================================

class Hsv
{

public:

	static void rgbToHsv(float const red, float const green, float const blue, float &hue, float &saturation, float &value);

	static void rgbToHsv(VectorArgb const & rgb, Vector & hsv );
	static void rgbToHsv(VectorRgba const & rgb, Vector & hsv );
};


//===================================================================

#endif
