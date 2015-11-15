// ======================================================================
//
// PackedQuaternion.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedQuaternion_H
#define INCLUDED_PackedQuaternion_H

// ======================================================================

#include "Archive/Archive.h"
#include "sharedMath/Quaternion.h"

// ======================================================================

class PackedQuaternion
{
public:
	PackedQuaternion();
	PackedQuaternion(Quaternion const &q);

	enum
	{
		cs_packedSize = 4
	};

	void set(Quaternion const &q);
	Quaternion const get() const;
	void unpack(Archive::ReadIterator &source);
	void pack(Archive::ByteStream &target) const;

private:
	int8 m_qw, m_qx, m_qy, m_qz;
};

// ======================================================================

inline PackedQuaternion::PackedQuaternion() :
	m_qw(127),
	m_qx(0),
	m_qy(0),
	m_qz(0)
{
}

// ----------------------------------------------------------------------

inline PackedQuaternion::PackedQuaternion(Quaternion const &q)
{
	set(q);
}

// ----------------------------------------------------------------------

inline void PackedQuaternion::set(Quaternion const &q)
{
	m_qw = static_cast<int8>(clamp(-1.f, q.w, 1.f)*127.f);
	m_qx = static_cast<int8>(clamp(-1.f, q.x, 1.f)*127.f);
	m_qy = static_cast<int8>(clamp(-1.f, q.y, 1.f)*127.f);
	m_qz = static_cast<int8>(clamp(-1.f, q.z, 1.f)*127.f);
}

// ----------------------------------------------------------------------

inline Quaternion const PackedQuaternion::get() const
{
	Quaternion q(m_qw/127.f, m_qx/127.f, m_qy/127.f, m_qz/127.f);
	q.normalize();
	return q;
}

// ----------------------------------------------------------------------

inline void PackedQuaternion::unpack(Archive::ReadIterator &source)
{
	Archive::get(source, m_qw);
	Archive::get(source, m_qx);
	Archive::get(source, m_qy);
	Archive::get(source, m_qz);
}

// ----------------------------------------------------------------------

inline void PackedQuaternion::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_qw);
	Archive::put(target, m_qx);
	Archive::put(target, m_qy);
	Archive::put(target, m_qz);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	inline void get(ReadIterator &source, PackedQuaternion &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	inline void put(ByteStream &target, PackedQuaternion const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedQuaternion_H

