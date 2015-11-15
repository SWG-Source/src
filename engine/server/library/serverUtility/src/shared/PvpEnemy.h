// ======================================================================
//
// PvpEnemy.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpEnemy_H
#define _PvpEnemy_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"

// ======================================================================

struct PvpEnemy
{
	PvpEnemy() :
		enemyId(NetworkId::cms_invalid),
		enemyFaction(0),
		expireTime(0)
	{
	}

	PvpEnemy(NetworkId const &id, uint32 faction, int time) :
		enemyId(id),
		enemyFaction(faction),
		expireTime(time)
	{
	}

	NetworkId enemyId;
	uint32 enemyFaction;
	int expireTime;
};

// ----------------------------------------------------------------------

inline bool operator==(PvpEnemy const &lhs, PvpEnemy const &rhs)
{
	return    lhs.expireTime == rhs.expireTime
	       && lhs.enemyId == rhs.enemyId
	       && lhs.enemyFaction == rhs.enemyFaction;
}

// ----------------------------------------------------------------------

namespace Archive
{
	class ByteStream;
	class ReadIterator;

	void put(Archive::ByteStream &target, PvpEnemy const &source);
	void get(Archive::ReadIterator &source, PvpEnemy &target);
}

// ======================================================================

#endif // _PvpEnemy_H

