// ChatOnAddModeratorToRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnAddModeratorToRoom_H
#define	_INCLUDED_ChatOnAddModeratorToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatOnAddModeratorToRoom : public GameNetworkMessage
{
public:
	ChatOnAddModeratorToRoom(const unsigned int resultCode, const unsigned int sequenceId, const ChatAvatarId & avatarId, const ChatAvatarId &granter, const std::string & roomName);
	explicit ChatOnAddModeratorToRoom(Archive::ReadIterator & source);
	~ChatOnAddModeratorToRoom();

	const ChatAvatarId &  getAvatarId    () const;
	const ChatAvatarId &  getGranterId   () const;
	const unsigned int    getResultCode  () const;
	const std::string &   getRoomName    () const;
	const unsigned int    getSequenceId  () const;

private:
	ChatOnAddModeratorToRoom & operator = (const ChatOnAddModeratorToRoom & rhs);
	ChatOnAddModeratorToRoom(const ChatOnAddModeratorToRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<ChatAvatarId>  granterId;
	Archive::AutoVariable<unsigned int>  resultCode;
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequenceId;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnAddModeratorToRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnAddModeratorToRoom::getGranterId() const
{
	return granterId.get();
}


//-----------------------------------------------------------------------

inline const unsigned int ChatOnAddModeratorToRoom::getResultCode() const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatOnAddModeratorToRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnAddModeratorToRoom::getSequenceId() const
{
	return sequenceId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnAddModeratorToRoom_H
