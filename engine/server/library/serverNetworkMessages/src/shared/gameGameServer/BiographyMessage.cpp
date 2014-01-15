//========================================================================
//
// BiographyMessage.cpp - tells Gameserver object baseline data has ended.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/BiographyMessage.h"

//-----------------------------------------------------------------------

BiographyMessage::BiographyMessage(const NetworkId &owner, const Unicode::String &bio) :
	GameNetworkMessage("BiographyMessage"),
	m_owner(owner),
	m_bio(bio)
{
	addVariable(m_owner);
	addVariable(m_bio);
}

//-----------------------------------------------------------------------

BiographyMessage::BiographyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("BiographyMessage"),
		m_owner(),
		m_bio()
{
	addVariable(m_owner);
	addVariable(m_bio);
	unpack(source);
}

//-----------------------------------------------------------------------

BiographyMessage::~BiographyMessage()
{
}

//-----------------------------------------------------------------------

