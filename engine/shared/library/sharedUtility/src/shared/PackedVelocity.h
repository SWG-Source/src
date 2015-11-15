// ======================================================================
//
// PackedVelocity.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedVelocity_H
#define INCLUDED_PackedVelocity_H

// ======================================================================

#include "Archive/Archive.h"
#include "sharedMath/Vector.h"
#include "sharedUtility/PackedUnitVector.h"

// ======================================================================

class PackedVelocity
{
public:
	PackedVelocity();
	PackedVelocity(Vector const &v);

	enum
	{
		cs_packedSize = 2+PackedUnitVector::cs_packedSize
	};

	void set(Vector const &v);
	Vector const get() const;
	void unpack(Archive::ReadIterator &source);
	void pack(Archive::ByteStream &target) const;

private:
	int16 m_speed;
	PackedUnitVector m_direction;
};

// ======================================================================

inline PackedVelocity::PackedVelocity() :
	m_speed(0),
	m_direction()
{
}

// ----------------------------------------------------------------------

inline PackedVelocity::PackedVelocity(Vector const &v)
{
	set(v);
}

// ----------------------------------------------------------------------

inline void PackedVelocity::set(Vector const &v)
{
	float const mag = v.magnitude();
	m_speed = static_cast<int16>(clamp(-512.f, mag, 512.f)*(32767.f/512.f));
	m_direction.set(mag ? v/mag : Vector::unitZ);
}

// ----------------------------------------------------------------------

inline Vector const PackedVelocity::get() const
{
	return m_direction.get() * (m_speed*(512.f/32767.f));
}

// ----------------------------------------------------------------------

inline void PackedVelocity::unpack(Archive::ReadIterator &source)
{
	Archive::get(source, m_speed);
	Archive::get(source, m_direction);
}

// ----------------------------------------------------------------------

inline void PackedVelocity::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_speed);
	Archive::put(target, m_direction);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	inline void get(ReadIterator &source, PackedVelocity &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	inline void put(ByteStream &target, PackedVelocity const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedVelocity_H

