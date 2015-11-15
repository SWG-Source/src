// ======================================================================
//
// Buff.h
//
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Adam Hunter
// ======================================================================

#ifndef	_INCLUDED_Buff_H
#define	_INCLUDED_Buff_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"

class Buff 
{
public:

	struct PackedBuff
	{
		uint32 endtime;
		float  value;
		uint32 duration;
		NetworkId  caster;
		uint32 stackCount;

		inline bool operator != (const Buff::PackedBuff& rhs) const;
		inline bool operator == (const Buff::PackedBuff& rhs) const;
	};

	Buff();
	Buff(std::string const &buffName, PackedBuff buffValues);
	Buff(std::string const &buffName, uint32 timestamp, float value, uint32 duration, NetworkId caster, uint32 stackCount);
	Buff(uint32 buffNameCrc, PackedBuff buffValues);
	Buff(uint32 buffNameCrc, uint32 timestamp, float value, uint32 duration, NetworkId caster = NetworkId::cms_invalid, uint32 stackCount = 1);
	~Buff();

	Buff &operator=(Buff const &rhs);

	PackedBuff getPackedBuffValue();
	static PackedBuff makePackedBuff(uint64 oldSchoolPackedBuff);

	void set(uint32 buffNameCrc, const PackedBuff &buffValues);

	std::string m_name;
	uint32 m_nameCrc;
	uint32 m_timestamp;
	float  m_value;
	uint32 m_duration;
	NetworkId  m_caster;
	uint32 m_stackCount;

};

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get (ReadIterator &source, Buff::PackedBuff &target);
	void put (ByteStream &target, const Buff::PackedBuff &source);
}


inline bool Buff::PackedBuff::operator != (const Buff::PackedBuff& rhs) const 
{ 
	return (endtime != rhs.endtime || value != rhs.value || duration != rhs.duration || caster != rhs.caster || stackCount != rhs.stackCount); 
}

inline bool Buff::PackedBuff::operator == (const Buff::PackedBuff& rhs) const
{ 
	return !(rhs != *this); 
}



// ======================================================================

#endif	// _INCLUDED_Buff_H

