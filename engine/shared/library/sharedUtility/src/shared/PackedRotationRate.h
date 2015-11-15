// ======================================================================
//
// PackedRotationRate.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PackedRotationRate_H
#define INCLUDED_PackedRotationRate_H

// ======================================================================

#include "Archive/Archive.h"

// ======================================================================

class PackedRotationRate
{
public:
	PackedRotationRate();
	PackedRotationRate(float rate);

	enum
	{
		cs_packedSize = 1
	};

	void set(float rate);
	float get() const;
	void unpack(Archive::ReadIterator &source);
	void pack(Archive::ByteStream &target) const;

private:
	int8 m_rate;
};

// ======================================================================

inline PackedRotationRate::PackedRotationRate() :
	m_rate(0)
{
}

// ----------------------------------------------------------------------

inline PackedRotationRate::PackedRotationRate(float rate)
{
	set(rate);
}

// ----------------------------------------------------------------------

inline void PackedRotationRate::set(float rate)
{
	m_rate = static_cast<int8>(clamp(-PI_OVER_2, rate, PI_OVER_2)*(127.f/PI_OVER_2));
}

// ----------------------------------------------------------------------

inline float PackedRotationRate::get() const
{
	return m_rate*(PI_OVER_2/127.f);
}

// ----------------------------------------------------------------------

inline void PackedRotationRate::unpack(Archive::ReadIterator &source)
{
	Archive::get(source, m_rate);
}

// ----------------------------------------------------------------------

inline void PackedRotationRate::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_rate);
}

// ======================================================================

namespace Archive
{

	// ----------------------------------------------------------------------

	inline void get(ReadIterator &source, PackedRotationRate &target)
	{
		target.unpack(source);
	}

	// ----------------------------------------------------------------------

	inline void put(ByteStream &target, PackedRotationRate const &source)
	{
		source.pack(target);
	}

	// ----------------------------------------------------------------------

}

// ======================================================================

#endif // INCLUDED_PackedRotationRate_H

