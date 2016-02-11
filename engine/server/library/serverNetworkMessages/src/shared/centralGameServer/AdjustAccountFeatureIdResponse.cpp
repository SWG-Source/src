// ======================================================================
//
// AdjustAccountFeatureIdResponse.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdResponse.h"

// ======================================================================

AdjustAccountFeatureIdResponse::AdjustAccountFeatureIdResponse(NetworkId const & requestingPlayer, uint32 gameServer, NetworkId const & targetPlayer, std::string const & targetPlayerDescription, StationId targetStationId, NetworkId const & targetItem, std::string const & targetItemDescription, uint32 gameCode, uint32 featureId, int oldValue, int newValue, unsigned int resultCode, bool resultCameFromSession, const char * sessionResultString /*= nullptr*/, const char * sessionResultText /*= nullptr*/) :
GameNetworkMessage("AdjustAccountFeatureIdResponse"),
m_requestingPlayer(requestingPlayer),
m_gameServer(gameServer),
m_targetPlayer(targetPlayer),
m_targetPlayerDescription(targetPlayerDescription),
m_targetStationId(targetStationId),
m_targetItem(targetItem),
m_targetItemDescription(targetItemDescription),
m_gameCode(gameCode),
m_featureId(featureId),
m_oldValue(oldValue),
m_newValue(newValue),
m_resultCode(resultCode),
m_resultCameFromSession(resultCameFromSession),
m_sessionResultString(),
m_sessionResultText()
{	
	if (sessionResultString)
		m_sessionResultString.set(std::string(sessionResultString));

	if (sessionResultText)
		m_sessionResultText.set(std::string(sessionResultText));

	addVariable(m_requestingPlayer);
	addVariable(m_gameServer);
	addVariable(m_targetPlayer);
	addVariable(m_targetPlayerDescription);
	addVariable(m_targetStationId);
	addVariable(m_targetItem);
	addVariable(m_targetItemDescription);
	addVariable(m_gameCode);
	addVariable(m_featureId);
	addVariable(m_oldValue);
	addVariable(m_newValue);
	addVariable(m_resultCode);
	addVariable(m_resultCameFromSession);
	addVariable(m_sessionResultString);
	addVariable(m_sessionResultText);
}

//-----------------------------------------------------------------------

AdjustAccountFeatureIdResponse::AdjustAccountFeatureIdResponse(Archive::ReadIterator & source) :
GameNetworkMessage("AdjustAccountFeatureIdResponse"),
m_requestingPlayer(),
m_gameServer(),
m_targetPlayer(),
m_targetPlayerDescription(),
m_targetStationId(),
m_targetItem(),
m_targetItemDescription(),
m_gameCode(),
m_featureId(),
m_oldValue(),
m_newValue(),
m_resultCode(),
m_resultCameFromSession(),
m_sessionResultString(),
m_sessionResultText()
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
	addVariable(m_oldValue);
	addVariable(m_newValue);
	addVariable(m_resultCode);
	addVariable(m_resultCameFromSession);
	addVariable(m_sessionResultString);
	addVariable(m_sessionResultText);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

AdjustAccountFeatureIdResponse::~AdjustAccountFeatureIdResponse()
{
}

// ======================================================================
