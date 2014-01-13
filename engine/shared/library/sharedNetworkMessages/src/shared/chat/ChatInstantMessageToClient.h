// ChatInstantMessageToClient.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatInstantMessageToClient_H
#define	_INCLUDED_ChatInstantMessageToClient_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatInstantMessageToClient : public GameNetworkMessage
{
public:
	ChatInstantMessageToClient(const ChatAvatarId & fromName, const Unicode::String & message, const Unicode::String & outOfBand);
	explicit ChatInstantMessageToClient(Archive::ReadIterator & source);
	~ChatInstantMessageToClient();

	const ChatAvatarId &     getFromName   () const;
	const Unicode::String &  getMessage    () const;
	const Unicode::String &  getOutOfBand  () const;

private:
	ChatInstantMessageToClient & operator = (const ChatInstantMessageToClient & rhs);
	ChatInstantMessageToClient(const ChatInstantMessageToClient & source);

	Archive::AutoVariable<ChatAvatarId>     fromName;
	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<Unicode::String>  outOfBand;
};

//-----------------------------------------------------------------------

inline const Unicode::String & ChatInstantMessageToClient::getMessage() const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatInstantMessageToClient::getFromName() const
{
	return fromName.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatInstantMessageToClient::getOutOfBand() const
{
	return outOfBand.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatInstantMessageToClient_H
