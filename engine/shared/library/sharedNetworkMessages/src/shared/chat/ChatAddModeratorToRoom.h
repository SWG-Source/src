// ChatAddModeratorToRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatAddModeratorToRoom_H
#define	_INCLUDED_ChatAddModeratorToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatAddModeratorToRoom : public GameNetworkMessage
{
public:
	ChatAddModeratorToRoom(const unsigned int sequenceId, const ChatAvatarId & avatarId, const std::string & roomName);
	ChatAddModeratorToRoom(Archive::ReadIterator & source);
	~ChatAddModeratorToRoom();

	const ChatAvatarId &  getAvatarId    () const;
	const std::string &   getRoomName    () const;
	const unsigned int    getSequenceId  () const;

private:
	ChatAddModeratorToRoom & operator = (const ChatAddModeratorToRoom & rhs);
	ChatAddModeratorToRoom(const ChatAddModeratorToRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequenceId;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatAddModeratorToRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatAddModeratorToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatAddModeratorToRoom::getSequenceId() const
{
	return sequenceId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatAddModeratorToRoom_H
