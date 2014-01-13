// ChatOnSendPersistentMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnSendPersistentMessage_H
#define	_INCLUDED_ChatOnSendPersistentMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnSendPersistentMessage : public GameNetworkMessage
{
public:
	ChatOnSendPersistentMessage(const unsigned int sequence, const unsigned int result);
	ChatOnSendPersistentMessage(Archive::ReadIterator & source);
	~ChatOnSendPersistentMessage();

	const unsigned int  getResult    () const;
	const unsigned int  getSequence  () const;

private:
	ChatOnSendPersistentMessage & operator = (const ChatOnSendPersistentMessage & rhs);
	ChatOnSendPersistentMessage(const ChatOnSendPersistentMessage & source);

	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendPersistentMessage::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendPersistentMessage::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnSendPersistentMessage_H

