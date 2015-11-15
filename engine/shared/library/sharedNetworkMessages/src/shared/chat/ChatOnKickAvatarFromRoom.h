// ChatOnKickAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

#ifndef	_INCLUDED_ChatOnKickAvatarFromRoom_H
#define	_INCLUDED_ChatOnKickAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatOnKickAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatOnKickAvatarFromRoom(const unsigned int resultCode, const ChatAvatarId & avatarId, const ChatAvatarId &removerId, const std::string & roomName);
	explicit ChatOnKickAvatarFromRoom(Archive::ReadIterator & source);
	~ChatOnKickAvatarFromRoom();

	const ChatAvatarId &  getAvatarId    () const;
	const ChatAvatarId &  getRemoverId   () const;
	const unsigned int    getResultCode  () const;
	const std::string &   getRoomName    () const;

private:
	ChatOnKickAvatarFromRoom & operator = (const ChatOnKickAvatarFromRoom & rhs);
	ChatOnKickAvatarFromRoom(const ChatOnKickAvatarFromRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<ChatAvatarId>  removerId;
	Archive::AutoVariable<unsigned int>  resultCode;
	Archive::AutoVariable<std::string>   roomName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnKickAvatarFromRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnKickAvatarFromRoom::getRemoverId() const
{
	return removerId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnKickAvatarFromRoom::getResultCode() const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatOnKickAvatarFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnKickAvatarFromRoom_H
