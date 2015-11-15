///////////////////////////////////////////////////////////////////////////////
//
// ChatOnDeleteAllPersistentMessages.cpp
// Copyright 2003, Sony Online Entertainment Inc., all rights reserved. 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef	INCLUDED_ChatOnDeleteAllPersistentMessages_H
#define	INCLUDED_ChatOnDeleteAllPersistentMessages_H

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------------

class ChatOnDeleteAllPersistentMessages : public GameNetworkMessage
{
public:
	ChatOnDeleteAllPersistentMessages(std::string const &targetName, bool const success);
	ChatOnDeleteAllPersistentMessages(Archive::ReadIterator & source);
	~ChatOnDeleteAllPersistentMessages();

	std::string const &getTargetName() const;
	bool const &isSuccess() const;

private:

	Archive::AutoVariable<std::string> m_targetName;
	Archive::AutoVariable<bool>        m_success;

	// Disabled

	ChatOnDeleteAllPersistentMessages();
	ChatOnDeleteAllPersistentMessages & operator = (const ChatOnDeleteAllPersistentMessages & rhs);
	ChatOnDeleteAllPersistentMessages(const ChatOnDeleteAllPersistentMessages & source);
};

///////////////////////////////////////////////////////////////////////////////

#endif	// INCLUDED_ChatOnDeleteAllPersistentMessages_H
