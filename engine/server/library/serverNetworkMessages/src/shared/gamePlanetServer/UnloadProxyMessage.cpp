// ======================================================================
//
// UnloadProxyMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/UnloadProxyMessage.h"

#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

UnloadProxyMessage::UnloadProxyMessage(NetworkId objectId, uint32 proxyGameServerId) :
		GameNetworkMessage("UnloadProxyMessage"),
		m_objectId(objectId),
		m_proxyGameServerId(proxyGameServerId)
{
	addVariable(m_objectId);
	addVariable(m_proxyGameServerId);
}

// ----------------------------------------------------------------------

UnloadProxyMessage::UnloadProxyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("UnloadProxyMessage"),
		m_objectId(),
		m_proxyGameServerId()
{
	addVariable(m_objectId);
	addVariable(m_proxyGameServerId);
	
	unpack(source);
}

//-----------------------------------------------------------------------

UnloadProxyMessage::~UnloadProxyMessage()
{
}

// ======================================================================
