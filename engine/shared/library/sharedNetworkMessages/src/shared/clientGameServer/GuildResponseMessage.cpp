// ======================================================================
//
// GuildResponseMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/GuildResponseMessage.h"

// ======================================================================

const char * const GuildResponseMessage::MessageType = "GuildResponseMessage";

//----------------------------------------------------------------------

GuildResponseMessage::GuildResponseMessage(NetworkId const &targetId, std::string const &guildName, std::string const &memberTitle) :
GameNetworkMessage(MessageType),
m_targetId(targetId),
m_guildName(guildName),
m_memberTitle(memberTitle)
{
	addVariable(m_targetId);
	addVariable(m_guildName);
	addVariable(m_memberTitle);
}

// ----------------------------------------------------------------------

GuildResponseMessage::GuildResponseMessage(Archive::ReadIterator &source) :
GameNetworkMessage(MessageType),
m_targetId(),
m_guildName(),
m_memberTitle()
{
	addVariable(m_targetId);
	addVariable(m_guildName);
	addVariable(m_memberTitle);
	unpack(source);
}

// ----------------------------------------------------------------------

GuildResponseMessage::~GuildResponseMessage()
{
}

// ======================================================================
