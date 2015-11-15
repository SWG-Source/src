// ChatPersistentMessageToClient.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatPersistentMessageToClient_H
#define	_INCLUDED_ChatPersistentMessageToClient_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientDataArchive.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientData.h"

//-----------------------------------------------------------------------

class ChatPersistentMessageToClient : public GameNetworkMessage
{
public:
	typedef ChatPersistentMessageToClientData Data;

	ChatPersistentMessageToClient(const unsigned int id, signed char status, const std::string & fromGameCode, const std::string & fromServerCode, const std::string & fromCharacterName, const Unicode::String & subject, const unsigned int timeStamp);
	ChatPersistentMessageToClient(const unsigned int id, signed char status, const std::string & fromGameCode, const std::string & fromServerCode, const std::string & fromCharacterName, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & oob, const unsigned int timeStamp);
	explicit ChatPersistentMessageToClient(Archive::ReadIterator &);
	~ChatPersistentMessageToClient();

	const Data &             getData               () const;

private:
	ChatPersistentMessageToClient & operator = (const ChatPersistentMessageToClient & rhs);
	ChatPersistentMessageToClient(const ChatPersistentMessageToClient & source);

	Archive::AutoVariable<Data>      data;
};

//----------------------------------------------------------------------

inline const ChatPersistentMessageToClient::Data & ChatPersistentMessageToClient::getData () const
{
	return data.get ();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatPersistentMessageToClient_H
