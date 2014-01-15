// ======================================================================
//
// AdjustAccountFeatureIdRequest.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdRequest.h"

// ======================================================================

AdjustAccountFeatureIdRequest::AdjustAccountFeatureIdRequest(NetworkId const & requestingPlayer, uint32 gameServer, NetworkId const & targetPlayer, std::string const & targetPlayerDescription, StationId targetStationId, NetworkId const & targetItem, std::string const & targetItemDescription, uint32 gameCode, uint32 featureId, int adjustment) :
	GameNetworkMessage("AdjustAccountFeatureIdRequest"),
	m_requestingPlayer(requestingPlayer),
	m_gameServer(gameServer),
	m_targetPlayer(targetPlayer),
	m_targetPlayerDescription(targetPlayerDescription),
	m_targetStationId(targetStationId),
	m_targetItem(targetItem),
	m_targetItemDescription(targetItemDescription),
	m_gameCode(gameCode),
	m_featureId(featureId),
	m_adjustment(adjustment)
{	
	addVariable(m_requestingPlayer);
	addVariable(m_gameServer);
	addVariable(m_targetPlayer);
	addVariable(m_targetPlayerDescription);
	addVariable(m_targetStationId);
	addVariable(m_targetItem);
	addVariable(m_targetItemDescription);
	addVariable(m_gameCode);
	addVariable(m_featureId);
	addVariable(m_adjustment);
}

//-----------------------------------------------------------------------

AdjustAccountFeatureIdRequest::AdjustAccountFeatureIdRequest(Archive::ReadIterator & source) :
	GameNetworkMessage("AdjustAccountFeatureIdRequest"),
	m_requestingPlayer(),
	m_gameServer(),
	m_targetPlayer(),
	m_targetPlayerDescription(),
	m_targetStationId(),
	m_targetItem(),
	m_targetItemDescription(),
	m_gameCode(),
	m_featureId(),
	m_adjustment()
{
	addVariable(m_requestingPlayer);
	addVariable(m_gameServer);
	addVariable(m_targetPlayer);
	addVariable(m_targetPlayerDescription);
	addVariable(m_targetStationId);
	addVariable(m_targetItem);
	addVariable(m_targetItemDescription);
	addVariable(m_gameCode);
	addVariable(m_featureId);
	addVariable(m_adjustment);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

AdjustAccountFeatureIdRequest::~AdjustAccountFeatureIdRequest()
{
}

// ======================================================================
