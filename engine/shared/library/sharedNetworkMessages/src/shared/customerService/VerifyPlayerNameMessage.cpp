// ============================================================================
//
// VerifyPlayerNameMessage.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ============================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/VerifyPlayerNameMessage.h"

// ============================================================================
//
// VerifyPlayerNameMessage
//
// ============================================================================

//-----------------------------------------------------------------------
VerifyPlayerNameMessage::VerifyPlayerNameMessage(const Unicode::String &playerName, const NetworkId &sourceNetworkId)
 : GameNetworkMessage("VerifyPlayerNameMessage")
 , m_playerName(playerName)
 , m_sourceNetworkId(sourceNetworkId)
{
	addVariable(m_playerName);
	addVariable(m_sourceNetworkId);
}

//-----------------------------------------------------------------------
VerifyPlayerNameMessage::VerifyPlayerNameMessage(Archive::ReadIterator &source)
 : GameNetworkMessage("VerifyPlayerNameMessage")
 , m_playerName()
 , m_sourceNetworkId()
{
	addVariable(m_playerName);
	addVariable(m_sourceNetworkId);
	unpack(source);
}

//-----------------------------------------------------------------------
VerifyPlayerNameMessage::~VerifyPlayerNameMessage()
{
}

//-----------------------------------------------------------------------
const Unicode::String &VerifyPlayerNameMessage::getPlayerName() const
{
	return m_playerName.get();
}

//-----------------------------------------------------------------------
const NetworkId &VerifyPlayerNameMessage::getSourceNetworkId() const
{
	return m_sourceNetworkId.get();
}

// ============================================================================
