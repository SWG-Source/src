// ======================================================================
//
// PackedPosition.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedPosition_H
#define INCLUDED_PackedPosition_H

// ======================================================================

#include "Archive/Archive.h"
#include "sharedMath/Vector.h"

// ======================================================================

class PackedPosition
{
public:
	PackedPosition();
	PackedPosition(Vector const &p);

	enum
	{
		cs_packedSize = 6
	};

	void set(Vector const &p);
	Vector const get() const;
	void unpack(Archive::ReadIterator &source);
	void pack(Archive::ByteStream &target) const;

private:
	int16 m_px, m_py, m_pz;
};

// ======================================================================

inline PackedPosition::PackedPosition() :
	m_px(0),
	m_py(0),
	m_pz(0)
{
}

// ----------------------------------------------------------------------

inline PackedPosition::PackedPosition(Vector const &p)
{
	set(p);
}

// ----------------------------------------------------------------------

inline void PackedPosition::set(Vector const &p)
{
	m_px = static_cast<int16>(clamp(-8000.f, p.x, 8000.f)*(32767.f/8000.f));
	m_py = static_cast<int16>(clamp(-8000.f, p.y, 8000.f)*(32767.f/8000.f));
	m_pz = static_cast<int16>(clamp(-8000.f, p.z, 8000.f)*(32767.f/8000.f));
}

// ----------------------------------------------------------------------

inline Vector const PackedPosition::get() const
{
	return Vector(m_px*(8000.f/32767.f), m_py*(8000.f/32767.f), m_pz*(8000.f/32767.f));
}

// ----------------------------------------------------------------------

inline void PackedPosition::unpack(Archive::ReadIterator &source)
{
	Archive::get(source, m_px);
	Archive::get(source, m_py);
	Archive::get(source, m_pz);
}

// ----------------------------------------------------------------------

inline void PackedPosition::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_px);
	Archive::put(target, m_py);
	Archive::put(target, m_pz);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	inline void get(ReadIterator &source, PackedPosition &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	inline void put(ByteStream &target, PackedPosition const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedPosition_H

