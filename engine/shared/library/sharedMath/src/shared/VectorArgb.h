// ======================================================================
//
// VectorArgb.h
// Portions Copyright 1998 Bootprint Entertainment
// Portions Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VectorArgb_H
#define INCLUDED_VectorArgb_H

// ======================================================================

class PackedArgb;

// ======================================================================

class VectorArgb
{
public:

	static const VectorArgb solidBlack;
	static const VectorArgb solidBlue; 
	static const VectorArgb solidCyan; 
	static const VectorArgb solidGray;
	static const VectorArgb solidGreen;
	static const VectorArgb solidRed;  
	static const VectorArgb solidMagenta;
	static const VectorArgb solidYellow;
	static const VectorArgb solidWhite;

	real a;
	real r;
	real g;
	real b;

	VectorArgb(void);
	VectorArgb(real newA, real newR, real newG, real newB);
	VectorArgb(const PackedArgb &argb);

	void set(real newA, real newR, real newG, real newB);
	float rgbIntensity() const;

	bool operator ==(const VectorArgb &rhs) const;
	bool operator !=(const VectorArgb &rhs) const;
	const VectorArgb operator *(float scalar) const;
	VectorArgb operator+(const VectorArgb &o) const { return VectorArgb(a+o.a, r+o.r, g+o.g, b+o.b); }

	static const VectorArgb linearInterpolate(const VectorArgb &color1, const VectorArgb &color2, real time);

	uint32  convertToUint32() const;
	uint32  convertToUint32NoClamp() const;
};

// ======================================================================

inline VectorArgb::VectorArgb(void)
: a(CONST_REAL(1)),
	r(CONST_REAL(0)),
	g(CONST_REAL(0)),
	b(CONST_REAL(0))
{
}

// ----------------------------------------------------------------------

inline VectorArgb::VectorArgb(real newA, real newR, real newG, real newB)
: a(newA),
	r(newR),
	g(newG),
	b(newB)
{
}

// ----------------------------------------------------------------------

inline void VectorArgb::set(real newA, real newR, real newG, real newB)
{
	a = newA;
	r = newR;
	g = newG;
	b = newB;
}

// ----------------------------------------------------------------------

inline bool VectorArgb::operator ==(const VectorArgb &rhs) const
{
	return ((a == rhs.a) && (r == rhs.r) && (g == rhs.g) && (b == rhs.b)); //lint !e777 // testing floats for equality
}

// ----------------------------------------------------------------------

inline bool VectorArgb::operator !=(const VectorArgb &rhs) const
{
	return !(*this == rhs);
}

// ----------------------------------------------------------------------

inline float VectorArgb::rgbIntensity() const
{
	return std::abs(r) * 0.30f + std::abs(g) * 0.59f + std::abs(b) * 0.11f;
}

// ----------------------------------------------------------------------
/**
 * Linearly interpolate between two VectorArgbs.
 * 
 * The time parameter should be between 0.0 and 1.0 inclusive in order to have
 * the result be between the two endpoints.  At time 0.0 the result will be
 * color1, and at time 1.0 the result will be color2.
 * 
 * @param color1  Starting endpoint
 * @param color2  Terminating endpoint
 * @param time   
 */

inline const VectorArgb VectorArgb::linearInterpolate(const VectorArgb &color1, const VectorArgb &color2, real time)
{
	return VectorArgb(color1.a + (color2.a - color1.a) * time, color1.r + (color2.r - color1.r) * time, color1.g + (color2.g - color1.g) * time, color1.b + (color2.b - color1.b) * time);
}

// ----------------------------------------------------------------------

inline uint32 VectorArgb::convertToUint32() const
{
	const uint32 a32 = clamp(static_cast<uint32>(0), static_cast<uint32>(a * 255.0f), static_cast<uint32>(255));
	const uint32 r32 = clamp(static_cast<uint32>(0), static_cast<uint32>(r * 255.0f), static_cast<uint32>(255));
	const uint32 g32 = clamp(static_cast<uint32>(0), static_cast<uint32>(g * 255.0f), static_cast<uint32>(255));
	const uint32 b32 = clamp(static_cast<uint32>(0), static_cast<uint32>(b * 255.0f), static_cast<uint32>(255));
	return (a32 << 24) | (r32 << 16) | (g32 << 8) | (b32 << 0);
}

// ----------------------------------------------------------------------

inline uint32 VectorArgb::convertToUint32NoClamp() const
{
	const uint32 a32 = static_cast<uint32>(a * 255.0f);
	const uint32 r32 = static_cast<uint32>(r * 255.0f);
	const uint32 g32 = static_cast<uint32>(g * 255.0f);
	const uint32 b32 = static_cast<uint32>(b * 255.0f);
	return (a32 << 24) | (r32 << 16) | (g32 << 8) | (b32 << 0);
}

// ----------------------------------------------------------------------

inline const VectorArgb VectorArgb::operator *(const float scalar) const
{
	return VectorArgb(a * scalar, r * scalar, g * scalar, b * scalar);
}

// ======================================================================

#endif
