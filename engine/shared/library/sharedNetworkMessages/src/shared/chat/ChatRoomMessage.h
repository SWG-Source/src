// ChatRoomMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRoomMessage_H
#define	_INCLUDED_ChatRoomMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatRoomMessage : public GameNetworkMessage
{
public:
	ChatRoomMessage(const unsigned int fromRoom, const ChatAvatarId & fromName, const Unicode::String & message, const Unicode::String & outOfBand);
	explicit ChatRoomMessage(Archive::ReadIterator & source);
	~ChatRoomMessage();

	const ChatAvatarId &     getFromName   () const;
	const unsigned int       getFromRoom   () const;
	const Unicode::String &  getMessage    () const;
	const Unicode::String &  getOutOfBand  () const;

private:
	ChatRoomMessage & operator = (const ChatRoomMessage & rhs);
	ChatRoomMessage(const ChatRoomMessage & source);

	Archive::AutoVariable<ChatAvatarId>     fromName;
	Archive::AutoVariable<unsigned int>     fromRoom;
	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<Unicode::String>  outOfBand;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatRoomMessage::getFromName() const
{
	return fromName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatRoomMessage::getFromRoom() const
{
	return fromRoom.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatRoomMessage::getMessage() const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatRoomMessage::getOutOfBand() const
{
	return outOfBand.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRoomMessage_H
