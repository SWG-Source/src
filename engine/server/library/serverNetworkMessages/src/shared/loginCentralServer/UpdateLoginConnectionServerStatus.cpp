// ======================================================================
//
// UpdateLoginConnectionServerStatus.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UpdateLoginConnectionServerStatus.h"

// ======================================================================

UpdateLoginConnectionServerStatus::UpdateLoginConnectionServerStatus(int id, uint16 publicPort, uint16 privatePort, int playerCount) :
		GameNetworkMessage("UpdateLoginConnectionServerStatus"),
		m_id(id),
		m_publicPort(publicPort),
		m_privatePort(privatePort),
		m_playerCount(playerCount)
{
	addVariable(m_id);
	addVariable(m_publicPort);
	addVariable(m_privatePort);
	addVariable(m_playerCount);
}

// ----------------------------------------------------------------------

UpdateLoginConnectionServerStatus::UpdateLoginConnectionServerStatus(Archive::ReadIterator & source) :
		GameNetworkMessage("UpdateLoginConnectionServerStatus"),
		m_id(0),
		m_publicPort(0),
		m_privatePort(0),
		m_playerCount(0)
{
	addVariable(m_id);
	addVariable(m_publicPort);
	addVariable(m_privatePort);
	addVariable(m_playerCount);	
	unpack(source);
}

// ======================================================================
