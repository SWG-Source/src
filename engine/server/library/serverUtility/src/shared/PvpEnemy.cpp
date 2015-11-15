// ======================================================================
//
// PvpEnemy.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/PvpEnemy.h"
#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"

// ======================================================================

namespace Archive
{

// ======================================================================

void put(Archive::ByteStream &target, PvpEnemy const &source)
{
	put(target, source.enemyId);
	put(target, source.enemyFaction);
	put(target, source.expireTime);
}

// ----------------------------------------------------------------------

void get(Archive::ReadIterator &source, PvpEnemy &target)
{
	get(source, target.enemyId);
	get(source, target.enemyFaction);
	get(source, target.expireTime);
}

// ======================================================================

}

// ======================================================================

