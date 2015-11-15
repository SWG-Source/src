// ======================================================================
//
// Buff.cpp
//
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Adam Hunter
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/Buff.h"

#include "Archive/Archive.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkIdArchive.h"
// ======================================================================

Buff::Buff() :
m_name(),
m_nameCrc(0),
m_timestamp(0),
m_value(0),
m_duration(0),
m_caster(NetworkId::cms_invalid),
m_stackCount(1)
{
}

Buff::Buff(std::string const &buffName, Buff::PackedBuff buffValues) :
m_name(buffName),
m_nameCrc(0),
m_timestamp(buffValues.endtime),
m_value(buffValues.value),
m_duration(buffValues.duration),
m_caster(buffValues.caster),
m_stackCount(buffValues.stackCount)
{
	m_nameCrc = Crc::calculate(buffName.c_str());
}

// ======================================================================

Buff::Buff(std::string const &buffName, uint32 timestamp, float value, uint32 duration, NetworkId caster, uint32 stackCount) :
m_name(buffName),
m_nameCrc(0),
m_timestamp(timestamp),
m_value(value),
m_duration(duration),
m_caster(caster),
m_stackCount(stackCount)
{
	m_nameCrc = Crc::calculate(buffName.c_str());
}

// ======================================================================

Buff::Buff(uint32 buffNameCrc, Buff::PackedBuff buffValues) :
m_name(""),
m_nameCrc(buffNameCrc),
m_timestamp(buffValues.endtime),
m_value(buffValues.value),
m_duration(buffValues.duration),
m_caster(buffValues.caster),
m_stackCount(buffValues.stackCount)
{
}

// ======================================================================

Buff::Buff(uint32 buffNameCrc, uint32 timestamp, float value, uint32 duration, NetworkId caster, uint32 stackCount) :
m_name(""),
m_nameCrc(buffNameCrc),
m_timestamp(timestamp),
m_value(value),
m_duration(duration),
m_caster(caster),
m_stackCount(stackCount)
{
}

// ======================================================================

Buff::~Buff()
{
}

// ======================================================================

Buff &Buff::operator=(Buff const &rhs)
{
	if (this != (&rhs))
	{
		m_name = rhs.m_name;
		m_nameCrc = rhs.m_nameCrc;
		m_timestamp = rhs.m_timestamp;
		m_value = rhs.m_value;
		m_duration = rhs.m_duration;
		m_caster = rhs.m_caster;
		m_stackCount = rhs.m_stackCount;
	}
	return *this;
}

// ======================================================================

Buff::PackedBuff Buff::getPackedBuffValue()
{
	PackedBuff result;
	result.endtime = m_timestamp;
	result.duration = m_duration;
	result.value = m_value;
	result.caster = m_caster;
	result.stackCount = m_stackCount;
	return result;
}

void Buff::set(uint32 buffNameCrc, const Buff::PackedBuff &buffValues)
{
	m_name.clear();
	m_nameCrc = buffNameCrc;
	m_timestamp = buffValues.endtime;
	m_value = buffValues.value;
	m_duration = buffValues.duration;
	m_caster = buffValues.caster;
	m_stackCount = buffValues.stackCount;
}
// ======================================================================

namespace Archive
{
	void get (ReadIterator &source, Buff::PackedBuff &target)
	{
		get(source, target.endtime);
		get(source, target.value);
		get(source, target.duration);
		get(source, target.caster);
		get(source, target.stackCount);
	}
	void put (ByteStream &target, const Buff::PackedBuff &source)
	{
		put(target, source.endtime);
		put(target, source.value);
		put(target, source.duration);
		put(target, source.caster);
		put(target, source.stackCount);
	}
}

Buff::PackedBuff Buff::makePackedBuff(uint64 oldSchoolPackedBuff)
{
	PackedBuff result;

	DEBUG_FATAL(sizeof(float) != sizeof(uint32), ("This buff struct won't work if float isn't 32 bits"));
	result.endtime = static_cast<uint32>(oldSchoolPackedBuff >> 32);
	uint32 tmp = static_cast<uint32>(oldSchoolPackedBuff & 0x00000000FFFFFFFF);
	// Yes, this really is taking 32 bits of memory and turning them into a float. -- ARH
	result.value = *(reinterpret_cast<float *>(&tmp));
	result.duration = 0;
	result.caster = NetworkId::cms_invalid;
	result.stackCount = 1;

	return result;
}
