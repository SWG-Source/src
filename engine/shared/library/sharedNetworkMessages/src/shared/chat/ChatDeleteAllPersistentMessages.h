///////////////////////////////////////////////////////////////////////////////
//
// ChatDeleteAllPersistentMessages.cpp
// Copyright 2003, Sony Online Entertainment Inc., all rights reserved. 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef	INCLUDED_ChatDeleteAllPersistentMessages_H
#define	INCLUDED_ChatDeleteAllPersistentMessages_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------------

class ChatDeleteAllPersistentMessages : public GameNetworkMessage
{
public:
	ChatDeleteAllPersistentMessages(NetworkId const &sourceNetworkId, NetworkId const &targetNetworkId);
	ChatDeleteAllPersistentMessages(Archive::ReadIterator & source);
	~ChatDeleteAllPersistentMessages();

	NetworkId const &getSourceNetworkId() const;
	NetworkId const &getTargetNetworkId() const;

private:

	Archive::AutoVariable<NetworkId> m_sourceNetworkId;
	Archive::AutoVariable<NetworkId> m_targetNetworkId;

	// Disabled

	ChatDeleteAllPersistentMessages();
	ChatDeleteAllPersistentMessages & operator = (const ChatDeleteAllPersistentMessages & rhs);
	ChatDeleteAllPersistentMessages(const ChatDeleteAllPersistentMessages & source);
};

///////////////////////////////////////////////////////////////////////////////

#endif	// INCLUDED_ChatDeleteAllPersistentMessages_H
