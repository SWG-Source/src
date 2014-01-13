// ChatOnDestroyRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnDestroyRoom_H
#define	_INCLUDED_ChatOnDestroyRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

class ChatOnDestroyRoom : public GameNetworkMessage
{
public:
	ChatOnDestroyRoom(const unsigned int sequence, const unsigned int resultCode, const unsigned int roomId, const ChatAvatarId & destroyer);
	ChatOnDestroyRoom(Archive::ReadIterator &);
	~ChatOnDestroyRoom();

	const ChatAvatarId &  getDestroyer     () const;
	const unsigned int    getResultCode    () const;
	const unsigned int    getRoomId        () const;
	const unsigned int    getSequence      () const;

private:
	ChatOnDestroyRoom & operator = (const ChatOnDestroyRoom & rhs);
	ChatOnDestroyRoom(const ChatOnDestroyRoom & source);

	Archive::AutoVariable<ChatAvatarId>  destroyer;
	Archive::AutoVariable<unsigned int>  resultCode;
	Archive::AutoVariable<unsigned int>  roomId;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const ChatAvatarId & ChatOnDestroyRoom::getDestroyer() const
{
	return destroyer.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnDestroyRoom::getResultCode() const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnDestroyRoom::getRoomId() const
{
	return roomId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnDestroyRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnDestroyRoom_H
