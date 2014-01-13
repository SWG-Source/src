// ChatOnInviteGroupToRoom.h
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

#ifndef	_INCLUDED_ChatOnInviteGroupToRoom_H
#define	_INCLUDED_ChatOnInviteGroupToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

//-----------------------------------------------------------------------

class ChatOnInviteGroupToRoom : public GameNetworkMessage
{
public:
	ChatOnInviteGroupToRoom(const unsigned int result, const std::string &roomName, const ChatAvatarId &invitor, const ChatAvatarId &invitee );
	ChatOnInviteGroupToRoom(Archive::ReadIterator & source);
	~ChatOnInviteGroupToRoom();

	const std::string  & getRoomName  () const;
	const ChatAvatarId & getInvitor   () const;
	const ChatAvatarId & getInvitee   () const;
	const unsigned int   getResult    () const;

private:
	ChatOnInviteGroupToRoom & operator = (const ChatOnInviteGroupToRoom & rhs);
	ChatOnInviteGroupToRoom(const ChatOnInviteGroupToRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<ChatAvatarId>  invitor;
	Archive::AutoVariable<ChatAvatarId>  invitee;
	Archive::AutoVariable<unsigned int>  result;
};

//-----------------------------------------------------------------------

inline const std::string & ChatOnInviteGroupToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnInviteGroupToRoom::getInvitor() const
{
	return invitor.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnInviteGroupToRoom::getInvitee() const
{
	return invitee.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnInviteGroupToRoom::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnInviteGroupToRoom_H

