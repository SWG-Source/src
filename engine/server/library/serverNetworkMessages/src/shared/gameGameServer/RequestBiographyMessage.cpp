//========================================================================
//
// RequestBiographyMessage.cpp - tells Gameserver object baseline data has ended.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RequestBiographyMessage.h"

//-----------------------------------------------------------------------

RequestBiographyMessage::RequestBiographyMessage(const NetworkId &owner) :
	GameNetworkMessage("RequestBiographyMessage"),
	m_owner(owner)
{
	addVariable(m_owner);
}

//-----------------------------------------------------------------------

RequestBiographyMessage::RequestBiographyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("RequestBiographyMessage"),
		m_owner()
{
	addVariable(m_owner);
	unpack(source);
}

//-----------------------------------------------------------------------

RequestBiographyMessage::~RequestBiographyMessage()
{
}

//-----------------------------------------------------------------------

