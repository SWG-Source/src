// ======================================================================
//
// GuildRequestMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GuildRequestMessage.h"

// ======================================================================

GuildRequestMessage::GuildRequestMessage(NetworkId const &targetId) :
	GameNetworkMessage("GuildRequestMessage"),
	m_targetId(targetId)
{
	addVariable(m_targetId);
}

// ----------------------------------------------------------------------

GuildRequestMessage::GuildRequestMessage(Archive::ReadIterator &source) :
	GameNetworkMessage("GuildRequestMessage")
{
	addVariable(m_targetId);
	unpack(source);
}

// ----------------------------------------------------------------------

GuildRequestMessage::~GuildRequestMessage()
{
}

// ======================================================================
