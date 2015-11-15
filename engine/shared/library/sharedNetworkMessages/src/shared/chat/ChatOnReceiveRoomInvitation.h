// ChatOnReceiveRoomInvitation.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnReceiveRoomInvitation_H
#define	_INCLUDED_ChatOnReceiveRoomInvitation_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnReceiveRoomInvitation : public GameNetworkMessage
{
public:
	ChatOnReceiveRoomInvitation(const std::string & roomName, const ChatAvatarId & invitorAvator);
	ChatOnReceiveRoomInvitation(Archive::ReadIterator & source);
	~ChatOnReceiveRoomInvitation();

	const ChatAvatarId &  getInvitorAvatar  () const;
	const std::string &   getRoomName       () const;

private:
	ChatOnReceiveRoomInvitation & operator = (const ChatOnReceiveRoomInvitation & rhs);
	ChatOnReceiveRoomInvitation(const ChatOnReceiveRoomInvitation & source);

	Archive::AutoVariable<ChatAvatarId>  invitorAvatar;
	Archive::AutoVariable<std::string>   roomName;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnReceiveRoomInvitation::getInvitorAvatar() const
{
	return invitorAvatar.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatOnReceiveRoomInvitation::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnReceiveRoomInvitation_H
