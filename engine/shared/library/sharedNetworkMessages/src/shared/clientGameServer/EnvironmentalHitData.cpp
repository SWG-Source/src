//======================================================================
//
// EnvironmentalHitData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/EnvironmentalHitData.h"

#include "Archive/Archive.h"

//======================================================================

EnvironmentalHitData::EnvironmentalHitData() :
nebulaId(0),
side(0),
damage(0.0f)
{
}

//----------------------------------------------------------------------

EnvironmentalHitData::EnvironmentalHitData(int _nebulaId, int8 _side, float _damage) :
nebulaId(_nebulaId),
side(_side),
damage(_damage)
{
}

//----------------------------------------------------------------------

void EnvironmentalHitData::get(Archive::ReadIterator & source)
{
	Archive::get(source, nebulaId);
	Archive::get(source, side);
	Archive::get(source, damage);
}

//----------------------------------------------------------------------

void EnvironmentalHitData::put(Archive::ByteStream & target) const
{
	Archive::put(target, nebulaId);
	Archive::put(target, side);
	Archive::put(target, damage);
}

//======================================================================
