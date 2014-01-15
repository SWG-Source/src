// ======================================================================
//
// BountyHunterTargetListMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "BountyHunterTargetListMessage.h"

// ======================================================================

BountyHunterTargetListMessage::BountyHunterTargetListMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("BountyHunterTargetListMessage"),
	m_targetList()
{
	AutoByteStream::addVariable(m_targetList);
	unpack(source);
}

BountyHunterTargetListMessage::BountyHunterTargetListMessage(
	const std::vector< std::pair< NetworkId, NetworkId > > & targetList
) :
	GameNetworkMessage("BountyHunterTargetListMessage"),
	m_targetList(targetList)
{
	AutoByteStream::addVariable(m_targetList);
}

BountyHunterTargetListMessage::~BountyHunterTargetListMessage()
{
}

// ======================================================================
