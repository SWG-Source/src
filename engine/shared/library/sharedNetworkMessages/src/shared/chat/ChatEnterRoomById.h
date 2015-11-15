// ChatEnterRoomById.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatEnterRoomById_H
#define	_INCLUDED_ChatEnterRoomById_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatEnterRoomById : public GameNetworkMessage
{
public:
	ChatEnterRoomById(unsigned int sequence, const unsigned int roomId, const std::string & roomName);
	ChatEnterRoomById(Archive::ReadIterator &);
	~ChatEnterRoomById();

	const unsigned int  getRoomId  () const;
	const std::string & getRoomName() const;
	const unsigned int  getSequence() const;

private:
	ChatEnterRoomById & operator = (const ChatEnterRoomById & rhs);
	ChatEnterRoomById(const ChatEnterRoomById & source);

	Archive::AutoVariable<unsigned int>  roomId;
	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatEnterRoomById::getRoomId() const
{
	return roomId.get();
}

//-----------------------------------------------------------------------

inline const std::string & ChatEnterRoomById::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatEnterRoomById::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatEnterRoomById_H
