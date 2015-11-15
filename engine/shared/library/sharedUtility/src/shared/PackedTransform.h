// ======================================================================
//
// PackedTransform.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedTransform_H
#define INCLUDED_PackedTransform_H

// ======================================================================

#include "Archive/Archive.h"
#include "sharedMath/Transform.h"
#include "sharedUtility/PackedQuaternion.h"
#include "sharedUtility/PackedPosition.h"

// ======================================================================

class PackedTransform
{
public:
	PackedTransform();
	PackedTransform(Transform const &tr);

	enum
	{
		cs_packedSize = 10
	};

	void set(Transform const &tr);
	Transform const get() const;
	void unpack(Archive::ReadIterator &source);
	void pack(Archive::ByteStream &target) const;

private:
	PackedQuaternion m_q;
	PackedPosition m_p;
};

// ======================================================================

inline PackedTransform::PackedTransform() :
	m_q(),
	m_p()
{
}

// ----------------------------------------------------------------------

inline PackedTransform::PackedTransform(Transform const &tr)
{
	set(tr);
}

// ----------------------------------------------------------------------

inline void PackedTransform::set(Transform const &tr)
{
	m_q.set(Quaternion(tr));
	m_p.set(tr.getPosition_p());
}

// ----------------------------------------------------------------------

inline Transform const PackedTransform::get() const
{
	Transform ret(Transform::IF_none);
	ret.setPosition_p(m_p.get());
	m_q.get().getTransformPreserveTranslation(&ret);
	return ret;
}

// ----------------------------------------------------------------------

inline void PackedTransform::unpack(Archive::ReadIterator &source)
{
	Archive::get(source, m_q);
	Archive::get(source, m_p);
}

// ----------------------------------------------------------------------

inline void PackedTransform::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_q);
	Archive::put(target, m_p);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	inline void get(ReadIterator &source, PackedTransform &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	inline void put(ByteStream &target, PackedTransform const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedTransform_H

