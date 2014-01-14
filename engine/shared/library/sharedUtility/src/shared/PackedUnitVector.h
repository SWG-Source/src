// ======================================================================
//
// PackedUnitVector.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedUnitVector_H
#define INCLUDED_PackedUnitVector_H

// ======================================================================

#include "Archive/Archive.h"
#include "sharedMath/Vector.h"

// ======================================================================

class PackedUnitVector
{
public:
	PackedUnitVector();
	PackedUnitVector(Vector const &v);

	enum
	{
		cs_packedSize = 2,
		cs_xSignMask = 0x8000u,
		cs_ySignMask = 0x4000u,
		cs_zSignMask = 0x2000u,
		cs_componentMask = 0x003fu,
		cs_componentMaskBits = 6
	};

	void set(Vector const &v);
	Vector const get() const;
	void unpack(Archive::ReadIterator &source);
	void pack(Archive::ByteStream &target) const;

private:
	uint16 m_value;
};

// ======================================================================

inline PackedUnitVector::PackedUnitVector() :
	m_value(0)
{
}

// ----------------------------------------------------------------------

inline PackedUnitVector::PackedUnitVector(Vector const &v)
{
	set(v);
}

// ----------------------------------------------------------------------

inline void PackedUnitVector::set(Vector const &v)
{
	m_value = 0;
	float x = v.x; if (x < 0.f) { x = -x; m_value |= cs_xSignMask; }
	float y = v.y; if (y < 0.f) { y = -y; m_value |= cs_ySignMask; }
	float z = v.z; if (z < 0.f) { z = -z; m_value |= cs_zSignMask; }
	float const w = static_cast<float>(cs_componentMask-1)/(x+y+z);
	m_value |= static_cast<uint16>(x*w)<<cs_componentMaskBits;
	m_value |= static_cast<uint16>(y*w);
}

// ----------------------------------------------------------------------

inline Vector const PackedUnitVector::get() const
{
	float x = static_cast<float>((m_value>>cs_componentMaskBits)&cs_componentMask);
	float y = static_cast<float>(m_value&cs_componentMask);
	float z = static_cast<float>(cs_componentMask)-x-y;
	if (m_value&cs_xSignMask) x = -x;
	if (m_value&cs_ySignMask) y = -y;
	if (m_value&cs_zSignMask) z = -z;
	Vector v(x, y, z);
	v.normalize();
	return v;
}

// ----------------------------------------------------------------------

inline void PackedUnitVector::unpack(Archive::ReadIterator &source)
{
	Archive::get(source, m_value);
}

// ----------------------------------------------------------------------

inline void PackedUnitVector::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_value);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	inline void get(ReadIterator &source, PackedUnitVector &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	inline void put(ByteStream &target, PackedUnitVector const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedUnitVector_H

