// ======================================================================
//
// PackedArgb.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedArgb_H
#define INCLUDED_PackedArgb_H

// ======================================================================

class VectorArgb;

// ======================================================================

class PackedArgb
{
private:

	static const real oo255;

public:

	static const PackedArgb solidBlack;
	static const PackedArgb solidBlue; 
	static const PackedArgb solidCyan; 
	static const PackedArgb solidGray;
	static const PackedArgb solidGreen;
	static const PackedArgb solidRed;  
	static const PackedArgb solidMagenta;
	static const PackedArgb solidYellow;
	static const PackedArgb solidWhite;
	
public:

	static PackedArgb const linearInterpolate(PackedArgb const & color1, PackedArgb const & color2, float t);

public:

	PackedArgb();
	PackedArgb(uint32 argb);
	PackedArgb(uint8 a, uint8 r, uint8 g, uint8 b);
	PackedArgb(const VectorArgb &color);

	uint32     getArgb() const;
	uint8      getA() const;
	uint8      getR() const;
	uint8      getG() const;
	uint8      getB() const;

	void       setArgb(uint32 Argb);
	void       setArgb(uint8 a, uint8 r, uint8 g, uint8 b);
	void       setArgb(const VectorArgb &argb);
	void       setA(uint8 a);
	void       setR(uint8 r);
	void       setG(uint8 g);
	void       setB(uint8 b);

	bool       operator==(const PackedArgb &rhs) const;
	bool       operator!=(const PackedArgb &rhs) const;

private:

	static uint32 convert(uint8 a, uint8 r, uint8 g, uint8 b);
	static uint32 convert(float a, float r, float g, float b);

private:

	// The representation of this cannot change without ramification.  At least
	// VertexBuffer assumes it can cast a uint32 argb to a PackedArgb without error.
	uint32 m_argb;
};

// ======================================================================

inline uint32 PackedArgb::convert(uint8 a, uint8 r, uint8 g, uint8 b)
{
	return
		static_cast<uint32>(a) << 24 |
		static_cast<uint32>(r) << 16 |
		static_cast<uint32>(g) <<  8 |
		static_cast<uint32>(b) <<  0;
}

// ----------------------------------------------------------------------

inline uint32 PackedArgb::convert(float a, float r, float g, float b)
{
	return convert(static_cast<uint8>(a * 255.0f), static_cast<uint8>(r * 255.0f), static_cast<uint8>(g * 255.0f),  static_cast<uint8>(b * 255.0f));
}

// ======================================================================
/**
 * Construct a default PackedArgb value.
 * All components will be set to 0.
 */
inline PackedArgb::PackedArgb()
: m_argb(0)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a PackedArgb value.
 * @param argb The initial component values.
 */

inline PackedArgb::PackedArgb(uint32 argb)
: m_argb(argb)
{
}

// ----------------------------------------------------------------------
/**
 * Construct a PackedArgb value.
 * @param a The initial alpha component.
 * @param r The initial red component.
 * @param g The initial green component.
 * @param a The initial blue component.
 */

inline PackedArgb::PackedArgb (uint8 a, uint8 r, uint8 g, uint8 b)
: m_argb(convert(a, r, g, b))
{
}

// ----------------------------------------------------------------------
/**
 * Return (a,r,g,b) value as a uint32 value with alpha component at MSB and blue component at LSB.
 * @return  the packed argb value
 */

inline uint32 PackedArgb::getArgb() const
{
	return m_argb;
}

// ----------------------------------------------------------------------
/**
 * Return the alpha component.
 * @return the alpha component.
 */

inline uint8 PackedArgb::getA() const
{
	return static_cast<uint8>((m_argb >> 24) & 0xff);
}

// ----------------------------------------------------------------------
/**
 * Return the red component.
 * @return the red component.
 */

inline uint8 PackedArgb::getR() const
{
	return static_cast<uint8>((m_argb >> 16) & 0xff);
}

// ----------------------------------------------------------------------
/**
 * Return the green component.
 * @return the green component.
 */

inline uint8 PackedArgb::getG() const
{
	return static_cast<uint8>((m_argb >> 8) & 0xff);
}

// ----------------------------------------------------------------------
/**
 * Return the blue component.
 * @return the blue component.
 */

inline uint8 PackedArgb::getB() const
{
	return static_cast<uint8>((m_argb >> 0) & 0xff);
}

// ----------------------------------------------------------------------
/**
 * Set the color.
 * @param a Alpha value.
 * @param r Red value.
 * @param g Green value.
 * @param b Blue value.
 */

inline void PackedArgb::setArgb(uint8 a, uint8 r, uint8 g, uint8 b)
{
	m_argb = convert(a, r, g, b);
}

// ----------------------------------------------------------------------
/**
 * Set the alpha and color.
 * @param a Alpha value.
 * @param r Red value.
 * @param g Green value.
 * @param b Blue value.
 */

inline void PackedArgb::setArgb(uint32 argb)
{
	m_argb = argb;
}

// ----------------------------------------------------------------------
/**
 * Set the alpha component.
 * @param a New alpha component value.
 */

inline void PackedArgb::setA(uint8 a)
{
	m_argb = (static_cast<uint32>(a) << 24) | (m_argb & 0x00ffffff);
}

// ----------------------------------------------------------------------
/**
 * Set the red component.
 * @param r New red component value.
 */

inline void PackedArgb::setR(uint8 r)
{
	m_argb = (static_cast<uint32>(r) << 16) | (m_argb & 0xff00ffff);
}

// ----------------------------------------------------------------------
/**
 * Set the red component.
 * @param g New green component value.
 */

inline void PackedArgb::setG(uint8 g)
{
	m_argb = (static_cast<uint32>(g) <<  8) | (m_argb & 0xffff00ff);
}

// ----------------------------------------------------------------------
/**
 * Set the red component.
 * @param b New blue component value.
 */

inline void PackedArgb::setB(uint8 b)
{
	m_argb = (static_cast<uint32>(b) <<  0) | (m_argb & 0xffffff00);
}

// ----------------------------------------------------------------------
/**
 * Compare two PackedArgb colors.
 * @return true if all the components are identical, otherwise false.
 */
inline bool PackedArgb::operator==(const PackedArgb &rhs) const
{
	return m_argb == rhs.m_argb;
}

// ----------------------------------------------------------------------
/**
 * Compare two PackedArgb colors.
 * @return true if any component is different, otherwise false.
 */

inline bool PackedArgb::operator !=(const PackedArgb& rhs) const
{
	return !(*this == rhs);
}

// ======================================================================

#endif
