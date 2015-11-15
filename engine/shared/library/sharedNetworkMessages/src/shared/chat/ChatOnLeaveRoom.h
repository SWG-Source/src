// ChatOnLeaveRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnLeaveRoom_H
#define	_INCLUDED_ChatOnLeaveRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatOnLeaveRoom : public GameNetworkMessage
{
public:
	ChatOnLeaveRoom(unsigned int sequence, const unsigned int resultCode, const unsigned int roomId, const ChatAvatarId & characterName);
	ChatOnLeaveRoom(Archive::ReadIterator &);
	~ChatOnLeaveRoom();

	const ChatAvatarId &  getCharacterName  () const;
	const unsigned int    getResultCode     () const;
	const unsigned int    getRoomId         () const;
	const unsigned int    getSequence       () const;

private:
	ChatOnLeaveRoom & operator = (const ChatOnLeaveRoom & rhs);
	ChatOnLeaveRoom(const ChatOnLeaveRoom & source);

	Archive::AutoVariable<ChatAvatarId>  characterName;
	Archive::AutoVariable<unsigned int>  resultCode;
	Archive::AutoVariable<unsigned int>  roomId;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnLeaveRoom::getCharacterName() const
{
	return characterName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnLeaveRoom::getResultCode() const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnLeaveRoom::getRoomId() const
{
	return roomId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnLeaveRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnLeaveRoom_H
