// ChatRequestPersistentMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatRequestPersistentMessage_H
#define	_INCLUDED_ChatRequestPersistentMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
//-----------------------------------------------------------------------

class ChatRequestPersistentMessage : public GameNetworkMessage
{
public:
	ChatRequestPersistentMessage(const unsigned int sequence, const unsigned int messageId);
	explicit ChatRequestPersistentMessage(Archive::ReadIterator &);
	~ChatRequestPersistentMessage();

	const unsigned int  getMessageId  () const;
	const unsigned int  getSequence   () const;

private:
	ChatRequestPersistentMessage & operator = (const ChatRequestPersistentMessage & rhs);
	ChatRequestPersistentMessage(const ChatRequestPersistentMessage & source);

	Archive::AutoVariable<unsigned int>  messageId;
	Archive::AutoVariable<unsigned int>  sequence;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatRequestPersistentMessage::getMessageId() const
{
	return messageId.get();
}

//-----------------------------------------------------------------------

inline const unsigned int ChatRequestPersistentMessage::getSequence() const
{
	return sequence.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatRequestPersistentMessage_H
