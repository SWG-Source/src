// ChatEnterRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatEnterRoom_H
#define	_INCLUDED_ChatEnterRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatEnterRoom : public GameNetworkMessage
{
public:
	ChatEnterRoom(unsigned int sequence, const std::string & roomName);
	ChatEnterRoom(Archive::ReadIterator &);
	~ChatEnterRoom();

	const std::string & getRoomName  () const;
	const unsigned int  getSequence  () const;

private:
	ChatEnterRoom & operator = (const ChatEnterRoom & rhs);
	ChatEnterRoom(const ChatEnterRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const std::string & ChatEnterRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatEnterRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatEnterRoom_H
