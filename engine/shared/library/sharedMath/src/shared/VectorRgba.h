// ======================================================================
//
// VectorRgba.h
// Portions Copyright 1998 Bootprint Entertainment
// Portions Copyright 2003-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VectorRgba_H
#define INCLUDED_VectorRgba_H

// ======================================================================

class PackedArgb;

// ======================================================================

class VectorRgba
{
public:

	static const VectorRgba solidBlack;
	static const VectorRgba solidBlue;
	static const VectorRgba solidCyan;
	static const VectorRgba solidGray;
	static const VectorRgba solidGreen;
	static const VectorRgba solidRed;
	static const VectorRgba solidMagenta;
	static const VectorRgba solidYellow;
	static const VectorRgba solidWhite;

public:

	float r;
	float g;
	float b;
	float a;

public:

	VectorRgba();
	VectorRgba(float newR, float newG, float newB, float newA);
	VectorRgba(PackedArgb const & argb);

	void set(float newR, float newG, float newB, float newA);
	float rgbIntensity() const;

	bool operator ==(VectorRgba const & rhs) const;
	bool operator !=(VectorRgba const & rhs) const;
	VectorRgba const operator *(float scalar) const;

	static VectorRgba const linearInterpolate(VectorRgba const & color1, VectorRgba const & color2, float time);

	uint32  convertToUint32() const;
};

// ======================================================================

inline VectorRgba::VectorRgba()
:
	r(0.0f),
	g(0.0f),
	b(0.0f),
	a(1.0f)
 {
}

// ----------------------------------------------------------------------

inline VectorRgba::VectorRgba(float const newR, float const newG, float const newB, float const newA)
:
	r(newR),
	g(newG),
	b(newB),
	a(newA)
 {
}

// ----------------------------------------------------------------------

inline void VectorRgba::set(float const newR, float const newG, float const newB, float const newA)
{
	r = newR;
	g = newG;
	b = newB;
	a = newA;
}

// ----------------------------------------------------------------------

inline bool VectorRgba::operator ==(VectorRgba const & rhs) const
{
	return ((r == rhs.r) && (g == rhs.g) && (b == rhs.b) && (a == rhs.a)); //lint !e777 // testing floats for equality
}

// ----------------------------------------------------------------------

inline bool VectorRgba::operator !=(VectorRgba const & rhs) const
{
	return !(*this == rhs);
}

// ----------------------------------------------------------------------

inline float VectorRgba::rgbIntensity() const
{
	return std::abs(r) * 0.30f + std::abs(g) * 0.59f + std::abs(b) * 0.11f;
}

// ----------------------------------------------------------------------
/**
 * Linearly interpolate between two VectorRgbas.
 *
 * The time parameter should be between 0.0 and 1.0 inclusive in order to have
 * the result be between the two endpoints.  At time 0.0 the result will be
 * color1, and at time 1.0 the result will be color2.
 *
 * @param color1  Starting endpoint
 * @param color2  Terminating endpoint
 * @param time
 */

inline VectorRgba const VectorRgba::linearInterpolate(VectorRgba const & color1, VectorRgba const & color2, float time)
{
	return VectorRgba(color1.r + (color2.r - color1.r) * time, color1.g + (color2.g - color1.g) * time, color1.b + (color2.b - color1.b) * time, color1.a + (color2.a - color1.a) * time);
}

// ----------------------------------------------------------------------

inline uint32 VectorRgba::convertToUint32() const
{
	const uint32 a32 = clamp(static_cast<uint32>(0), static_cast<uint32>(a * 255.0f), static_cast<uint32>(255));
	const uint32 r32 = clamp(static_cast<uint32>(0), static_cast<uint32>(r * 255.0f), static_cast<uint32>(255));
	const uint32 g32 = clamp(static_cast<uint32>(0), static_cast<uint32>(g * 255.0f), static_cast<uint32>(255));
	const uint32 b32 = clamp(static_cast<uint32>(0), static_cast<uint32>(b * 255.0f), static_cast<uint32>(255));
	return (a32 << 24) | (r32 << 16) | (g32 << 8) | (b32 << 0);
}

// ----------------------------------------------------------------------

inline VectorRgba const VectorRgba::operator *(float const scalar) const
{
	return VectorRgba(r * scalar, g * scalar, b * scalar, a * scalar);
}

// ======================================================================

#endif
