//========================================================================
//
// PvpData.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_PvpData_H
#define INCLUDED_PvpData_H

namespace PvpStatusFlags
{
	enum
	{
		YouCanAttack   = (1<<0),
		CanAttackYou   = (1<<1),
		Declared       = (1<<2),
		HasEnemies     = (1<<3),
		IsPlayer       = (1<<4),
		IsEnemy        = (1<<5),
		WillBeDeclared = (1<<6),
		WasDeclared    = (1<<7),
		YouCanHelp     = (1<<8)
	};
}	// namespace PvpStatusFlags

namespace PvpData
{
	// for optimization, these crc are hard coded here (rather
	// than using a crc string object to calculate the crc value);
	// the words "imperial", "rebel", and "neutral" are intentionally
	// put here so that someone searching for these words will end
	// up here and use these methods rather than writing their own
	inline uint32 getImperialFactionId()
	{
		return 3679112276lu; // "imperial"
	}

	inline bool isImperialFactionId(uint32 crc)
	{
		return crc == 3679112276lu; // "imperial"
	}

	inline uint32 getRebelFactionId()
	{
		return 370444368lu; // "rebel"
	}

	inline bool isRebelFactionId(uint32 crc)
	{
		return crc == 370444368lu; // "rebel"
	}

	inline uint32 getNeutralFactionId()
	{
		return 0lu; // "neutral"
	}

	inline bool isNeutralFactionId(uint32 crc)
	{
		return crc == 0lu; // "neutral"
	}

	// factional presence tracking grid size
	inline int getGcwFactionalPresenceGridSize()
	{
		return 500;
	}
}

//========================================================================

#endif	// INCLUDED_PvpData_H

