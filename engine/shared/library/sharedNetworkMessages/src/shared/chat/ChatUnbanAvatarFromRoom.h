// ChatUnbanAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_ChatUnbanAvatarFromRoom_H
#define	_INCLUDED_ChatUnbanAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatUnbanAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatUnbanAvatarFromRoom(const unsigned int sequence, const ChatAvatarId & avatarId, const std::string & roomName);
	ChatUnbanAvatarFromRoom(Archive::ReadIterator & source);
	~ChatUnbanAvatarFromRoom();

	const ChatAvatarId &  getAvatarId  () const;
	const std::string &   getRoomName  () const;
	const unsigned int    getSequence  () const;

private:
	ChatUnbanAvatarFromRoom & operator = (const ChatUnbanAvatarFromRoom & rhs);
	ChatUnbanAvatarFromRoom(const ChatUnbanAvatarFromRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId; //avatar to ban
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatUnbanAvatarFromRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatUnbanAvatarFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatUnbanAvatarFromRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatUnbanAvatarFromRoom_H
