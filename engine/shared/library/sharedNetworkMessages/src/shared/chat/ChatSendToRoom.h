// ChatSendToRoom.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatSendToRoom_H
#define	_INCLUDED_ChatSendToRoom_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatSendToRoom : public GameNetworkMessage
{
public:
	ChatSendToRoom(const unsigned int sequence, const unsigned int roomId, const Unicode::String & message, const Unicode::String & outOfBand);
	ChatSendToRoom(Archive::ReadIterator & source);
	~ChatSendToRoom();

	const Unicode::String &  getMessage     () const;
	const Unicode::String &  getOutOfBand   () const;
	const unsigned int       getRoomId      () const;
	const unsigned int       getSequence    () const;

private:
	ChatSendToRoom & operator = (const ChatSendToRoom & rhs);
	ChatSendToRoom(const ChatSendToRoom & source);

	Archive::AutoVariable<Unicode::String>  message;
	Archive::AutoVariable<Unicode::String>  outOfBand;
	Archive::AutoVariable<unsigned int>     roomId;
	Archive::AutoVariable<unsigned int>     sequence;
};

//-----------------------------------------------------------------------

inline const Unicode::String & ChatSendToRoom::getMessage() const
{
	return message.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ChatSendToRoom::getOutOfBand() const
{
	return outOfBand.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatSendToRoom::getRoomId() const
{
	return roomId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatSendToRoom::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatSendToRoom_H
