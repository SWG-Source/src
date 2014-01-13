// ChatOnRemoveModeratorFromRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnRemoveModeratorFromRoom_H
#define	_INCLUDED_ChatOnRemoveModeratorFromRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"

//-----------------------------------------------------------------------

class ChatOnRemoveModeratorFromRoom : public GameNetworkMessage
{
public:
	ChatOnRemoveModeratorFromRoom(const unsigned int resultCode, const unsigned int sequenceId, const ChatAvatarId & avatarId, const ChatAvatarId &removerId, const std::string & roomName);
	explicit ChatOnRemoveModeratorFromRoom(Archive::ReadIterator & source);
	~ChatOnRemoveModeratorFromRoom();

	const ChatAvatarId &  getAvatarId    () const;
	const ChatAvatarId &  getRemoverId   () const;
	const unsigned int    getResultCode  () const;
	const std::string &   getRoomName    () const;
	const unsigned int    getSequenceId  () const;

private:
	ChatOnRemoveModeratorFromRoom & operator = (const ChatOnRemoveModeratorFromRoom & rhs);
	ChatOnRemoveModeratorFromRoom(const ChatOnRemoveModeratorFromRoom & source);

	Archive::AutoVariable<ChatAvatarId>  avatarId;
	Archive::AutoVariable<ChatAvatarId>  removerId;
	Archive::AutoVariable<unsigned int>  resultCode;
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequenceId;

};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnRemoveModeratorFromRoom::getAvatarId() const
{
	return avatarId.get();
}

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnRemoveModeratorFromRoom::getRemoverId() const
{
	return removerId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnRemoveModeratorFromRoom::getResultCode() const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatOnRemoveModeratorFromRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnRemoveModeratorFromRoom::getSequenceId() const
{
	return sequenceId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnRemoveModeratorFromRoom_H
