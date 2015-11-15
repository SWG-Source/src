// ======================================================================
//
// GuildResponseMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GuildResponseMessage_H
#define INCLUDED_GuildResponseMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class GuildResponseMessage: public GameNetworkMessage
{
public:

	static const char * const MessageType;

	GuildResponseMessage(NetworkId const &targetId, std::string const &guildName, std::string const &memberTitle);
	explicit GuildResponseMessage(Archive::ReadIterator &source);
	virtual ~GuildResponseMessage();

	NetworkId const &getTargetId() const;
	std::string const &getGuildName() const;
	std::string const &getMemberTitle() const;
	
private:
	GuildResponseMessage();
	GuildResponseMessage(GuildResponseMessage const &);
	GuildResponseMessage &operator=(GuildResponseMessage const &);

private:
	Archive::AutoVariable<NetworkId> m_targetId;
	Archive::AutoVariable<std::string> m_guildName;
	Archive::AutoVariable<std::string> m_memberTitle;
};

// ----------------------------------------------------------------------

inline NetworkId const &GuildResponseMessage::getTargetId() const
{
	return m_targetId.get();
}

// ----------------------------------------------------------------------

inline std::string const &GuildResponseMessage::getGuildName() const
{
	return m_guildName.get();
}

// ----------------------------------------------------------------------

inline std::string const &GuildResponseMessage::getMemberTitle() const
{
	return m_memberTitle.get();
}

// ======================================================================

#endif	// _INCLUDED_GuildResponseMessage_H

