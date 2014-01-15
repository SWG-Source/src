// ======================================================================
//
// UpdateConnectionServerStatus.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UpdateConnectionServerStatus.h"

// ======================================================================

UpdateConnectionServerStatus::UpdateConnectionServerStatus(uint16 publicPort, uint16 privatePort) :
		GameNetworkMessage("UpdateConnectionServerStatus"),
		m_publicPort(publicPort),
		m_privatePort(privatePort)
{
	addVariable(m_publicPort);
	addVariable(m_privatePort);
}

// ----------------------------------------------------------------------

UpdateConnectionServerStatus::UpdateConnectionServerStatus(Archive::ReadIterator & source) :
		GameNetworkMessage("UpdateConnectionServerStatus"),
		m_publicPort(0),
		m_privatePort(0)
{
	addVariable(m_publicPort);
	addVariable(m_privatePort);
	
	unpack(source);
}

// ======================================================================
