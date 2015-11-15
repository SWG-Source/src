// ChatOnInviteToRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnInviteToRoom_H
#define	_INCLUDED_ChatOnInviteToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

//-----------------------------------------------------------------------

class ChatOnInviteToRoom : public GameNetworkMessage
{
public:
	ChatOnInviteToRoom(const unsigned int result, const std::string &roomName, const ChatAvatarId &invitor, const ChatAvatarId &invitee );
	ChatOnInviteToRoom(Archive::ReadIterator & source);
	~ChatOnInviteToRoom();

	const std::string  & getRoomName  () const;
	const ChatAvatarId & getInvitor   () const;
	const ChatAvatarId & getInvitee   () const;
	const unsigned int   getResult    () const;

private:
	ChatOnInviteToRoom & operator = (const ChatOnInviteToRoom & rhs);
	ChatOnInviteToRoom(const ChatOnInviteToRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<ChatAvatarId>  invitor;
	Archive::AutoVariable<ChatAvatarId>  invitee;
	Archive::AutoVariable<unsigned int>  result;

};

//-----------------------------------------------------------------------

inline const std::string & ChatOnInviteToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnInviteToRoom::getInvitor() const
{
	return invitor.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnInviteToRoom::getInvitee() const
{
	return invitee.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnInviteToRoom::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnInviteToRoom_H

