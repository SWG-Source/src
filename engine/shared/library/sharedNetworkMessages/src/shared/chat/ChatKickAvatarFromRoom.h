// ChatKickAvatarFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

#ifndef	_INCLUDED_ChatKickAvatarFromRoom_H
#define	_INCLUDED_ChatKickAvatarFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatKickAvatarFromRoom : public GameNetworkMessage
{
public:
	ChatKickAvatarFromRoom(const ChatAvatarId & avatarId, const std::string & roomName);
	explicit ChatKickAvatarFromRoom(Archive::ReadIterator & source);
	~ChatKickAvatarFromRoom();

	const ChatAvatarId &  getAvatarId    () const;
	const std::string &   getRoomName    () const;

private:
	ChatKickAvatarFromRoom & operator = (const ChatKickAvatarFromRoom & rhs);
	ChatKickAvatarFromRoom(const ChatKickAvatarFromRoom & source);

	Archive::AutoVariable<ChatAvatarId> m_avatarId;
	Archive::AutoVariable<std::string>  m_roomName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatKickAvatarFromRoom::getAvatarId() const
{
	return m_avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatKickAvatarFromRoom::getRoomName() const
{
	return m_roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatKickAvatarFromRoom_H
