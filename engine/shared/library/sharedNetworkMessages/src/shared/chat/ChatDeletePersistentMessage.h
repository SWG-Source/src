// ChatDeletePersistentMessage.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatDeletePersistentMessage_H
#define	_INCLUDED_ChatDeletePersistentMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatDeletePersistentMessage : public GameNetworkMessage
{
public:
	explicit ChatDeletePersistentMessage(const unsigned int messageId);
	ChatDeletePersistentMessage(Archive::ReadIterator &);
	~ChatDeletePersistentMessage();

	const unsigned int  getMessageId  () const;

private:
	ChatDeletePersistentMessage & operator = (const ChatDeletePersistentMessage & rhs);
	ChatDeletePersistentMessage(const ChatDeletePersistentMessage & source);

	Archive::AutoVariable<unsigned int>  messageId;
};

//-----------------------------------------------------------------------

inline const unsigned int ChatDeletePersistentMessage::getMessageId() const
{
	return messageId.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatDeletePersistentMessage_H
