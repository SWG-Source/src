// ======================================================================
//
// ExhcnageListCredits.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/ExchangeListCreditsMessage.h"

// ======================================================================

ExchangeListCreditsMessage::ExchangeListCreditsMessage(NetworkId const &actorId, uint32 credits, uint32 processId) :
	GameNetworkMessage("ExchangeListCreditsMessage"),
	m_actorId(actorId),
	m_credits(credits),
	m_processId(processId)
{
	addVariable(m_actorId);
	addVariable(m_credits);
	addVariable(m_processId);
}

// ----------------------------------------------------------------------

ExchangeListCreditsMessage::ExchangeListCreditsMessage(Archive::ReadIterator & source) :
	GameNetworkMessage("ExchangeListCreditsMessage"),
	m_actorId(),
	m_credits(),
	m_processId()
{
	addVariable(m_actorId);
	addVariable(m_credits);
	addVariable(m_processId);
	unpack(source);
}

// ----------------------------------------------------------------------

ExchangeListCreditsMessage::~ExchangeListCreditsMessage()
{
}

// ======================================================================

