// ChatOnSendInstantMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatOnSendInstantMessage_H
#define	_INCLUDED_ChatOnSendInstantMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnSendInstantMessage : public GameNetworkMessage
{
public:
	ChatOnSendInstantMessage(const unsigned int sequence, const unsigned int result);
	ChatOnSendInstantMessage(Archive::ReadIterator & source);
	~ChatOnSendInstantMessage();

	const unsigned int  getResult    () const;
	const unsigned int  getSequence  () const;

private:
	ChatOnSendInstantMessage & operator = (const ChatOnSendInstantMessage & rhs);
	ChatOnSendInstantMessage(const ChatOnSendInstantMessage & source);

	Archive::AutoVariable<unsigned int>  result;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendInstantMessage::getResult() const
{
	return result.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatOnSendInstantMessage::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnSendInstantMessage_H

