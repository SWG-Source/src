//======================================================================
//
// NebulaLightningData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NebulaLightningData.h"

#include "Archive/Archive.h"
#include "sharedMathArchive/VectorArchive.h"

//======================================================================

NebulaLightningData::NebulaLightningData() :
lightningId(0),
nebulaId(0),
syncStampStart(0),
syncStampEnd(0),
endpoint0(),
endpoint1()
{
}

//----------------------------------------------------------------------

void NebulaLightningData::get(Archive::ReadIterator & source)
{
	Archive::get(source, lightningId);
	Archive::get(source, nebulaId);
	Archive::get(source, syncStampStart);
	Archive::get(source, syncStampEnd);
	Archive::get(source, endpoint0);
	Archive::get(source, endpoint1);
}

//----------------------------------------------------------------------

void NebulaLightningData::put(Archive::ByteStream & target) const
{
	Archive::put(target, lightningId);
	Archive::put(target, nebulaId);
	Archive::put(target, syncStampStart);
	Archive::put(target, syncStampEnd);
	Archive::put(target, endpoint0);
	Archive::put(target, endpoint1);
}

//======================================================================
