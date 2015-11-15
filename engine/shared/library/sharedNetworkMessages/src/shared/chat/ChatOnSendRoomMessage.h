// ChatOnSendRoomMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnSendRoomMessage_H
#define	_INCLUDED_ChatOnSendRoomMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnSendRoomMessage : public GameNetworkMessage
{
public:
	ChatOnSendRoomMessage(const unsigned int sequence, const unsigned int result);
	explicit ChatOnSendRoomMessage(Archive::ReadIterator & source);
	~ChatOnSendRoomMessage();

	const unsigned int  getResult    () const;
	const unsigned int  getSequence  () const;

private:
	ChatOnSendRoomMessage & operator = (const ChatOnSendRoomMessage & rhs);
	ChatOnSendRoomMessage(const ChatOnSendRoomMessage & source);

	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendRoomMessage::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendRoomMessage::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnSendRoomMessage_H
