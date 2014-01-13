// ======================================================================
//
// GuildRequestMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GuildRequestMessage_H
#define INCLUDED_GuildRequestMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class GuildRequestMessage: public GameNetworkMessage
{
public:
	GuildRequestMessage(NetworkId const &targetId);
	explicit GuildRequestMessage(Archive::ReadIterator &source);
	virtual ~GuildRequestMessage();

	NetworkId const &getTargetId() const;

private:
	GuildRequestMessage(GuildRequestMessage const &);
	GuildRequestMessage &operator=(GuildRequestMessage const &);

private:
	Archive::AutoVariable<NetworkId> m_targetId;
};

// ----------------------------------------------------------------------

inline NetworkId const &GuildRequestMessage::getTargetId() const
{
	return m_targetId.get();
}

// ======================================================================

#endif	// _INCLUDED_GuildRequestMessage_H

