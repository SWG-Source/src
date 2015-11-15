// ChatBanAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_ChatBanAvatarFromRoom_H
#define	_INCLUDED_ChatBanAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatBanAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatBanAvatarFromRoom(const unsigned int sequence, const ChatAvatarId & avatarId, const std::string & roomName);
	ChatBanAvatarFromRoom(Archive::ReadIterator & source);
	~ChatBanAvatarFromRoom();

	const ChatAvatarId &  getAvatarId  () const;
	const std::string &   getRoomName  () const;
	const unsigned int    getSequence  () const;

private:
	ChatBanAvatarFromRoom & operator = (const ChatBanAvatarFromRoom & rhs);
	ChatBanAvatarFromRoom(const ChatBanAvatarFromRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId; //avatar to ban
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatBanAvatarFromRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatBanAvatarFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatBanAvatarFromRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatBanAvatarFromRoom_H
