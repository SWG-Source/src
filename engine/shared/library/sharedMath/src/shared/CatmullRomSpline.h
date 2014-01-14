// ============================================================================
//
// CatmullRomSpline.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_CatmullRomSpline_H
#define INCLUDED_CatmullRomSpline_H

class Vector;

//-----------------------------------------------------------------------------
class CatmullRomSpline
{
public:

	static void getCatmullRomSplinePoint(float const c1x, float const c1y, float const c2x, float const c2y, float const c3x, float const c3y, float const c4x, float const c4y, float const t, float &resultX, float &resultY);
	static void getCatmullRomSplinePoint3d(Vector const &c1, Vector const &c2, Vector const &c3, Vector const &c4, float const t, Vector &result);

private:

	// Disabled

	CatmullRomSpline();
	CatmullRomSpline(CatmullRomSpline const &);
	~CatmullRomSpline();
	CatmullRomSpline &operator =(CatmullRomSpline const &);
};

// ============================================================================

#endif // INCLUDED_CatmullRomSpline_H
