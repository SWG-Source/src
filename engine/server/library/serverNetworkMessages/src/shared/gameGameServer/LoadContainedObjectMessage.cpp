//========================================================================
//
// LoadContainedObjectMessage.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/LoadContainedObjectMessage.h"

#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

LoadContainedObjectMessage::LoadContainedObjectMessage(const NetworkId & containerId, const NetworkId &objectId) :
	GameNetworkMessage("LoadContainedObjectMessage"),
	m_containerId(containerId),
	m_objectId(objectId)
{
	addVariable(m_containerId);
	addVariable(m_objectId);
}

//-----------------------------------------------------------------------

LoadContainedObjectMessage::LoadContainedObjectMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("LoadContainedObjectMessage"),
		m_containerId(),
		m_objectId()
{
	addVariable(m_containerId);
	addVariable(m_objectId);
	unpack(source);
}

//-----------------------------------------------------------------------

LoadContainedObjectMessage::~LoadContainedObjectMessage()
{
}

//-----------------------------------------------------------------------

