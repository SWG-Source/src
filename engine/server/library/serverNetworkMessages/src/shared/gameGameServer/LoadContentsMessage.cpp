//========================================================================
//
// LoadContentsMessage.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoadContentsMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

LoadContentsMessage::LoadContentsMessage(const NetworkId & containerId) :
	GameNetworkMessage("LoadContentsMessage"),
	m_containerId(containerId)
{
	addVariable(m_containerId);
}

//-----------------------------------------------------------------------

LoadContentsMessage::LoadContentsMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("LoadContentsMessage"),
		m_containerId()
{
	addVariable(m_containerId);
	unpack(source);
}

//-----------------------------------------------------------------------

LoadContentsMessage::~LoadContentsMessage()
{
}

//-----------------------------------------------------------------------

