// ChatRemoveModeratorFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRemoveModeratorFromRoom_H
#define	_INCLUDED_ChatRemoveModeratorFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatRemoveModeratorFromRoom : public GameNetworkMessage
{
public:
	ChatRemoveModeratorFromRoom(const unsigned int sequenceId, const ChatAvatarId & avatarId, const std::string & roomName);
	ChatRemoveModeratorFromRoom(Archive::ReadIterator & source);
	~ChatRemoveModeratorFromRoom();

	const ChatAvatarId &  getAvatarId    () const;
	const std::string &   getRoomName    () const;
	const unsigned int    getSequenceId  () const;

private:
	ChatRemoveModeratorFromRoom & operator = (const ChatRemoveModeratorFromRoom & rhs);
	ChatRemoveModeratorFromRoom(const ChatRemoveModeratorFromRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequenceId;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatRemoveModeratorFromRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatRemoveModeratorFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatRemoveModeratorFromRoom::getSequenceId() const
{
	return sequenceId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRemoveModeratorFromRoom_H
