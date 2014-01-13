// ChatInviteGroupToRoom.h
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

#ifndef	_INCLUDED_ChatInviteGroupToRoom_H
#define	_INCLUDED_ChatInviteGroupToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatInviteGroupToRoom : public GameNetworkMessage
{
public:
	ChatInviteGroupToRoom(const ChatAvatarId & avatarId, const std::string & roomName);
	ChatInviteGroupToRoom(Archive::ReadIterator & source);
	~ChatInviteGroupToRoom();

	const ChatAvatarId &  getAvatarId  () const;
	const std::string &   getRoomName  () const;

private:
	ChatInviteGroupToRoom & operator = (const ChatInviteGroupToRoom & rhs);
	ChatInviteGroupToRoom(const ChatInviteGroupToRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<std::string>   roomName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatInviteGroupToRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatInviteGroupToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatInviteGroupToRoom_H
