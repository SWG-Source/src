// ======================================================================
//
// AccountFeatureIdResponse.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AccountFeatureIdResponse.h"

// ======================================================================

AccountFeatureIdResponse::AccountFeatureIdResponse(NetworkId const & requester, uint32 gameServer, NetworkId const & target, StationId targetStationId, uint32 gameCode, AccountFeatureIdRequest::RequestReason requestReason, unsigned int resultCode, bool resultCameFromSession, std::map<uint32, int> const & featureIds, std::map<uint32, std::string> const & sessionFeatureIdsData, const char * sessionResultString /*= nullptr*/, const char * sessionResultText /*= nullptr*/) :
	GameNetworkMessage("AccountFeatureIdResponse"),
	m_requester(requester),
	m_gameServer(gameServer),
	m_target(target),
	m_targetStationId(targetStationId),
	m_gameCode(gameCode),
	m_requestReason(static_cast<int8>(requestReason)),
	m_resultCode(resultCode),
	m_resultCameFromSession(resultCameFromSession),
	m_featureIds(featureIds),
	m_sessionFeatureIdsData(sessionFeatureIdsData),
	m_sessionResultString(),
	m_sessionResultText()
{
	if (sessionResultString)
		m_sessionResultString.set(std::string(sessionResultString));

	if (sessionResultText)
		m_sessionResultText.set(std::string(sessionResultText));

	addVariable(m_requester);
	addVariable(m_gameServer);
	addVariable(m_target);
	addVariable(m_targetStationId);
	addVariable(m_gameCode);
	addVariable(m_requestReason);
	addVariable(m_resultCode);
	addVariable(m_resultCameFromSession);
	addVariable(m_featureIds);
	addVariable(m_sessionFeatureIdsData);
	addVariable(m_sessionResultString);
	addVariable(m_sessionResultText);
}

//-----------------------------------------------------------------------

AccountFeatureIdResponse::AccountFeatureIdResponse(Archive::ReadIterator & source) :
	GameNetworkMessage("AccountFeatureIdResponse"),
	m_requester(),
	m_gameServer(),
	m_target(),
	m_targetStationId(),
	m_gameCode(),
	m_requestReason(),
	m_resultCode(),
	m_resultCameFromSession(),
	m_featureIds(),
	m_sessionFeatureIdsData(),
	m_sessionResultString(),
	m_sessionResultText()
{
	addVariable(m_requester);
	addVariable(m_gameServer);
	addVariable(m_target);
	addVariable(m_targetStationId);
	addVariable(m_gameCode);
	addVariable(m_requestReason);
	addVariable(m_resultCode);
	addVariable(m_resultCameFromSession);
	addVariable(m_featureIds);
	addVariable(m_sessionFeatureIdsData);
	addVariable(m_sessionResultString);
	addVariable(m_sessionResultText);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

AccountFeatureIdResponse::~AccountFeatureIdResponse()
{
}

// ======================================================================
