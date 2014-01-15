// ======================================================================
//
// PlayedTimeAccumMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/PlayedTimeAccumMessage.h"

// ======================================================================

PlayedTimeAccumMessage::PlayedTimeAccumMessage(NetworkId const &networkId, float playedTimeAccum) :
	GameNetworkMessage("PlayedTimeAccumMessage"),
	m_networkId(networkId),
	m_playedTimeAccum(playedTimeAccum)
{
	addVariable(m_networkId);
	addVariable(m_playedTimeAccum);
}

// ----------------------------------------------------------------------

PlayedTimeAccumMessage::PlayedTimeAccumMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("PlayedTimeAccumMessage"),
	m_networkId(),
	m_playedTimeAccum(0.0f)
{
	addVariable(m_networkId);
	addVariable(m_playedTimeAccum);
	unpack(source);
}

// ======================================================================

