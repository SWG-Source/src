// ChatOnCreateRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnCreateRoom_H
#define	_INCLUDED_ChatOnCreateRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ChatRoomData.h"
#include "sharedNetworkMessages/ChatRoomDataArchive.h"

//-----------------------------------------------------------------------

class ChatOnCreateRoom : public GameNetworkMessage
{
public:
	ChatOnCreateRoom(const unsigned int sequence, const unsigned int resultCode, const ChatRoomData & roomData);
	ChatOnCreateRoom(Archive::ReadIterator &);
	~ChatOnCreateRoom();

	const unsigned int    getResultCode    () const;
	const ChatRoomData &  getRoomData      () const;
	const unsigned int    getSequence      () const;

private:
	ChatOnCreateRoom & operator = (const ChatOnCreateRoom & rhs);
	ChatOnCreateRoom(const ChatOnCreateRoom & source);

	Archive::AutoVariable<unsigned int>  resultCode;
	Archive::AutoVariable<ChatRoomData>  roomData;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatOnCreateRoom::getResultCode() const
{
	return resultCode.get();
}

//-----------------------------------------------------------------------

inline const ChatRoomData & ChatOnCreateRoom::getRoomData() const
{
	return roomData.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnCreateRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnCreateRoom_H
