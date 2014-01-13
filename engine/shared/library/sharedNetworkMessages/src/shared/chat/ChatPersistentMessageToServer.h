// ChatPersistentMessageToServer.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatPersistentMessageToServer_H
#define	_INCLUDED_ChatPersistentMessageToServer_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------


namespace ChatPersistentMessageToServerNamespace {
	const unsigned int MAX_MESSAGE_SIZE = 4000;
}

class ChatPersistentMessageToServer : public GameNetworkMessage
{
public:
	ChatPersistentMessageToServer(const unsigned int sequence, const ChatAvatarId & toCharacter, const Unicode::String & subject, const Unicode::String & message, const Unicode::String & outOfBand);
	ChatPersistentMessageToServer(Archive::ReadIterator &);
	~ChatPersistentMessageToServer();

	const Unicode::String &  getMessage          () const;
	const Unicode::String &  getOutOfBand        () const;
	const unsigned int       getSequence         () const;
	const Unicode::String &  getSubject          () const;
	const ChatAvatarId &     getToCharacterName  () const;

private:
	ChatPersistentMessageToServer & operator = (const ChatPersistentMessageToServer & rhs);
	ChatPersistentMessageToServer(const ChatPersistentMessageToServer & source);

	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<Unicode::String>  outOfBand;
	Archive::AutoVariable<unsigned int>     sequence;
	Archive::AutoVariable<Unicode::String>  subject;
	Archive::AutoVariable<ChatAvatarId>     toCharacterName;
};

//-----------------------------------------------------------------------

inline const Unicode::String & ChatPersistentMessageToServer::getMessage() const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatPersistentMessageToServer::getOutOfBand() const
{
	return outOfBand.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatPersistentMessageToServer::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatPersistentMessageToServer::getSubject() const
{
	return subject.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatPersistentMessageToServer::getToCharacterName() const
{
	return toCharacterName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatPersistentMessageToServer_H
