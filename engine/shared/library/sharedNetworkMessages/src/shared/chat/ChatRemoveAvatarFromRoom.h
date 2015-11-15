// ChatRemoveAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRemoveAvatarFromRoom_H
#define	_INCLUDED_ChatRemoveAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatRemoveAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatRemoveAvatarFromRoom(const ChatAvatarId & avatarId, const std::string & roomName);
	explicit ChatRemoveAvatarFromRoom(Archive::ReadIterator & source);
	~ChatRemoveAvatarFromRoom();

	const ChatAvatarId &  getAvatarId  () const;
	const std::string &   getRoomName  () const;

private:
	ChatRemoveAvatarFromRoom & operator = (const ChatRemoveAvatarFromRoom & rhs);
	ChatRemoveAvatarFromRoom(const ChatRemoveAvatarFromRoom & source);

	Archive::AutoVariable<ChatAvatarId> avatarId;
	Archive::AutoVariable<std::string>  roomName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatRemoveAvatarFromRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatRemoveAvatarFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRemoveAvatarFromRoom_H
