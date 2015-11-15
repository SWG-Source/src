// ======================================================================
//
// FeatureIdTransactionRequest.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/FeatureIdTransactionRequest.h"

// ======================================================================

FeatureIdTransactionRequest::FeatureIdTransactionRequest(uint32 gameServer, StationId stationId, NetworkId const & player) :
	GameNetworkMessage("FeatureIdTransactionRequest"),
	m_gameServer(gameServer),
	m_stationId(stationId),
	m_player(player)
{
	addVariable(m_gameServer);
	addVariable(m_stationId);
	addVariable(m_player);
}

//-----------------------------------------------------------------------

FeatureIdTransactionRequest::FeatureIdTransactionRequest(Archive::ReadIterator & source) :
	GameNetworkMessage("FeatureIdTransactionRequest"),
	m_gameServer(),
	m_stationId(),
	m_player()
{
	addVariable(m_gameServer);
	addVariable(m_stationId);
	addVariable(m_player);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

FeatureIdTransactionRequest::~FeatureIdTransactionRequest()
{
}

// ======================================================================
