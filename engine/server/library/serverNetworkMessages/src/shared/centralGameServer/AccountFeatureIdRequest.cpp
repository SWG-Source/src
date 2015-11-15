// ======================================================================
//
// AccountFeatureIdRequest.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"

// ======================================================================

AccountFeatureIdRequest::AccountFeatureIdRequest(NetworkId const & requester, uint32 gameServer, NetworkId const & target, StationId targetStationId, uint32 gameCode, RequestReason requestReason) :
	GameNetworkMessage("AccountFeatureIdRequest"),
	m_requester(requester),
	m_gameServer(gameServer),
	m_target(target),
	m_targetStationId(targetStationId),
	m_gameCode(gameCode),
	m_requestReason(static_cast<int8>(requestReason))
{	
	addVariable(m_requester);
	addVariable(m_gameServer);
	addVariable(m_target);
	addVariable(m_targetStationId);
	addVariable(m_gameCode);
	addVariable(m_requestReason);
}

//-----------------------------------------------------------------------

AccountFeatureIdRequest::AccountFeatureIdRequest(Archive::ReadIterator & source) :
	GameNetworkMessage("AccountFeatureIdRequest"),
	m_requester(),
	m_gameServer(),
	m_target(),
	m_targetStationId(),
	m_gameCode(),
	m_requestReason()
{
	addVariable(m_requester);
	addVariable(m_gameServer);
	addVariable(m_target);
	addVariable(m_targetStationId);
	addVariable(m_gameCode);
	addVariable(m_requestReason);

	unpack(source); //lint !e1506 Call to virtual from constructor
}

//-----------------------------------------------------------------------

AccountFeatureIdRequest::~AccountFeatureIdRequest()
{
}

// ======================================================================
