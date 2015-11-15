// ChatInviteAvatarToRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatInviteAvatarToRoom_H
#define	_INCLUDED_ChatInviteAvatarToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatInviteAvatarToRoom : public GameNetworkMessage
{
public:
	ChatInviteAvatarToRoom(const ChatAvatarId & avatarId, const std::string & roomName);
	ChatInviteAvatarToRoom(Archive::ReadIterator & source);
	~ChatInviteAvatarToRoom();

	const ChatAvatarId &  getAvatarId  () const;
	const std::string &   getRoomName  () const;

private:
	ChatInviteAvatarToRoom & operator = (const ChatInviteAvatarToRoom & rhs);
	ChatInviteAvatarToRoom(const ChatInviteAvatarToRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<std::string>   roomName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatInviteAvatarToRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatInviteAvatarToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatInviteAvatarToRoom_H
