//======================================================================
//
// NebulaLightningHitData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NebulaLightningHitData.h"

#include "Archive/Archive.h"

//======================================================================

NebulaLightningHitData::NebulaLightningHitData() :
lightningId(0),
side(0),
damage(0.0f)
{
}

//----------------------------------------------------------------------

NebulaLightningHitData::NebulaLightningHitData(uint16 _lightningId, int8 _side, float _damage) :
lightningId(_lightningId),
side(_side),
damage(_damage)
{
}

//----------------------------------------------------------------------

void NebulaLightningHitData::get(Archive::ReadIterator & source)
{
	Archive::get(source, lightningId);
	Archive::get(source, side);
	Archive::get(source, damage);
}

//----------------------------------------------------------------------

void NebulaLightningHitData::put(Archive::ByteStream & target) const
{
	Archive::put(target, lightningId);
	Archive::put(target, side);
	Archive::put(target, damage);
}

//======================================================================
