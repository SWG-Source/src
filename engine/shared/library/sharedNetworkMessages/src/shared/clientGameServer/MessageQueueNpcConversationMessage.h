//========================================================================
//
// MessageQueueNpcConversationMessage.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueNpcConversationMessage_H
#define INCLUDED_MessageQueueNpcConversationMessage_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueNpcConversationMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueNpcConversationMessage(void);
	virtual ~MessageQueueNpcConversationMessage();

	void                    setNpcMessage(const Unicode::String & response);
	const Unicode::String & getNpcMessage(void) const;

private:
	Unicode::String m_message;
};


inline MessageQueueNpcConversationMessage::MessageQueueNpcConversationMessage()
{
}	// MessageQueueNpcConversationMessage::MessageQueueNpcConversationMessage()

inline void MessageQueueNpcConversationMessage::setNpcMessage(const Unicode::String & message)
{
	m_message = message;
}	// MessageQueueNpcConversationMessage::addNpcResponse

inline const Unicode::String & MessageQueueNpcConversationMessage::getNpcMessage(void) const
{
	return m_message;
}	// MessageQueueNpcConversationMessage::getNpcResponse


#endif	// INCLUDED_MessageQueueNpcConversationMessage_H
