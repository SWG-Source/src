// ChatQueryRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatQueryRoom_H
#define	_INCLUDED_ChatQueryRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatQueryRoom : public GameNetworkMessage
{
public:
	ChatQueryRoom(unsigned int sequence, const std::string & roomName);
	ChatQueryRoom(Archive::ReadIterator &);
	~ChatQueryRoom();

	const std::string & getRoomName  () const;
	const unsigned int  getSequence  () const;

private:
	ChatQueryRoom & operator = (const ChatQueryRoom & rhs);
	ChatQueryRoom(const ChatQueryRoom & source);

	Archive::AutoVariable<std::string>   roomName;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const std::string & ChatQueryRoom::getRoomName() const
{
	return roomName.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatQueryRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatQueryRoom_H
