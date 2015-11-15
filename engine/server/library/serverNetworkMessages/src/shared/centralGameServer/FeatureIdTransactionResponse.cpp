// ======================================================================
//
// FeatureIdTransactionResponse.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FeatureIdTransactionResponse.h"

// ======================================================================

FeatureIdTransactionResponse::FeatureIdTransactionResponse(uint32 gameServer, NetworkId const & player, std::map<std::string, int> const & transactions) :
	GameNetworkMessage("FeatureIdTransactionResponse"),
	m_gameServer(gameServer),
	m_player(player),
	m_transactions(transactions)
{
	addVariable(m_gameServer);
	addVariable(m_player);
	addVariable(m_transactions);
}

//-----------------------------------------------------------------------

FeatureIdTransactionResponse::FeatureIdTransactionResponse(Archive::ReadIterator & source) :
	GameNetworkMessage("FeatureIdTransactionResponse"),
	m_gameServer(),
	m_player(),
	m_transactions()
{
	addVariable(m_gameServer);
	addVariable(m_player);
	addVariable(m_transactions);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

FeatureIdTransactionResponse::~FeatureIdTransactionResponse()
{
}

// ======================================================================
