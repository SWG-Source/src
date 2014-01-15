// ======================================================================
//
// BountyHunterTargetMessage.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "BountyHunterTargetMessage.h"

// ======================================================================

BountyHunterTargetMessage::BountyHunterTargetMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("BountyHunterTargetMessage"),
	m_objectId(),
	m_targetId()
{
	AutoByteStream::addVariable(m_objectId);
	AutoByteStream::addVariable(m_targetId);
	unpack(source);
}

BountyHunterTargetMessage::BountyHunterTargetMessage(
	const NetworkId & objectId,
	const NetworkId & targetId
) :
	GameNetworkMessage("BountyHunterTargetMessage"),
	m_objectId(objectId),
	m_targetId(targetId)
{
	AutoByteStream::addVariable(m_objectId);
	AutoByteStream::addVariable(m_targetId);
}

BountyHunterTargetMessage::~BountyHunterTargetMessage()
{
}

// ======================================================================
