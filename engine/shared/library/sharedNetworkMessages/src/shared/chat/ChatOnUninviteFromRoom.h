// ChatOnUninviteFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnUninviteFromRoom_H
#define	_INCLUDED_ChatOnUninviteFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

//-----------------------------------------------------------------------

class ChatOnUninviteFromRoom : public GameNetworkMessage
{
public:
	ChatOnUninviteFromRoom(const unsigned int sequence, const unsigned int result, const std::string &roomName, const ChatAvatarId &invitor, const ChatAvatarId &invitee );
	ChatOnUninviteFromRoom(Archive::ReadIterator & source);
	~ChatOnUninviteFromRoom();

	const std::string  & getRoomName  () const;
	const ChatAvatarId & getInvitor   () const;
	const ChatAvatarId & getInvitee   () const;
	const unsigned int   getResult    () const;
	const unsigned int   getSequence  () const;

private:
	ChatOnUninviteFromRoom & operator = (const ChatOnUninviteFromRoom & rhs);
	ChatOnUninviteFromRoom(const ChatOnUninviteFromRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<ChatAvatarId>  invitor;
	Archive::AutoVariable<ChatAvatarId>  invitee;
	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;

};

//-----------------------------------------------------------------------

inline const std::string & ChatOnUninviteFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnUninviteFromRoom::getInvitor() const
{
	return invitor.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnUninviteFromRoom::getInvitee() const
{
	return invitee.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnUninviteFromRoom::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnUninviteFromRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnUninviteFromRoom_H

