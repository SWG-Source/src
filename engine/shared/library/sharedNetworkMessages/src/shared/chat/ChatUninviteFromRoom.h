// ChatUninviteFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatUninviteFromRoom_H
#define	_INCLUDED_ChatUninviteFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatUninviteFromRoom : public GameNetworkMessage
{
public:
	ChatUninviteFromRoom(const unsigned int sequence, const ChatAvatarId & avatar, const std::string & roomName);
	explicit ChatUninviteFromRoom(Archive::ReadIterator & source);
	~ChatUninviteFromRoom();

	const ChatAvatarId &  getAvatar    () const;
	const std::string &   getRoomName  () const;
	const unsigned int    getSequence  () const;

private:
	ChatUninviteFromRoom & operator = (const ChatUninviteFromRoom & rhs);
	ChatUninviteFromRoom(const ChatUninviteFromRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatar;
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatUninviteFromRoom::getAvatar() const
{
	return avatar.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatUninviteFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatUninviteFromRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatUninviteFromRoom_H
