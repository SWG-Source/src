// ============================================================================
//
// VerifyPlayerNameResponseMessage.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ============================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/VerifyPlayerNameResponseMessage.h"

// ============================================================================
//
// VerifyPlayerNameResponseMessage
//
// ============================================================================

//-----------------------------------------------------------------------
VerifyPlayerNameResponseMessage::VerifyPlayerNameResponseMessage(bool const valid, Unicode::String const &playerName)
 : GameNetworkMessage("VerifyPlayerNameResponseMessage")
 , m_valid(valid)
 , m_playerName(playerName)
{
	addVariable(m_valid);
	addVariable(m_playerName);
}

//-----------------------------------------------------------------------
VerifyPlayerNameResponseMessage::VerifyPlayerNameResponseMessage(Archive::ReadIterator &source)
 : GameNetworkMessage("VerifyPlayerNameResponseMessage")
 , m_valid(false)
 , m_playerName()
{
	addVariable(m_valid);
	addVariable(m_playerName);
	unpack(source);
}

//-----------------------------------------------------------------------
VerifyPlayerNameResponseMessage::~VerifyPlayerNameResponseMessage()
{
}

//-----------------------------------------------------------------------
Unicode::String const &VerifyPlayerNameResponseMessage::getPlayerName() const
{
	return m_playerName.get();
}

//-----------------------------------------------------------------------
bool VerifyPlayerNameResponseMessage::isValid() const
{
	return m_valid.get();
}

// ============================================================================
