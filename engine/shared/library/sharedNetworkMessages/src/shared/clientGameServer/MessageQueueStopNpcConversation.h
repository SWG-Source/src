//========================================================================
//
// MessageQueueStopNpcConversation.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#ifndef INCLUDED_MessageQueueStopNpcConversation_H
#define INCLUDED_MessageQueueStopNpcConversation_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

#include "StringId.h"

//-----------------------------------------------------------------------

class MemoryBlockManager;
class MessageQueueStopNpcConversation : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueStopNpcConversation();
	MessageQueueStopNpcConversation(NetworkId const & npc, StringId const & finalMessageId, Unicode::String const & finalProsePackage, Unicode::String const & finalResponse);
	virtual ~MessageQueueStopNpcConversation();

	MessageQueueStopNpcConversation&	operator=	(const MessageQueueStopNpcConversation & source);
	MessageQueueStopNpcConversation(const MessageQueueStopNpcConversation & source);

	const NetworkId & getNpc() const;
	void setNpc(NetworkId const & npc);

	void setFinalMessageId(StringId const & finalMessage);
	StringId const & getFinalMessageId() const;

	void setFinalMessageProse(Unicode::String const & finalMessage);
	Unicode::String const & getFinalMessageProse() const;

	void setFinalResponse(Unicode::String const & finalResponse);
	Unicode::String const & getFinalResponse() const;

private:
	NetworkId m_npc;
	StringId m_finalMessageId;
	Unicode::String m_finalMessageProse;
	Unicode::String m_finalResponse;
};

//-----------------------------------------------------------------------

inline MessageQueueStopNpcConversation::MessageQueueStopNpcConversation()
{
}	// MessageQueueStopNpcConversation::MessageQueueStopNpcConversation()

//-----------------------------------------------------------------------

inline MessageQueueStopNpcConversation::MessageQueueStopNpcConversation(const MessageQueueStopNpcConversation & source) :
	MessageQueue::Data(),
	m_npc(source.m_npc),
	m_finalMessageId(source.m_finalMessageId),
	m_finalMessageProse(source.m_finalMessageProse),
	m_finalResponse(source.m_finalResponse)
{
}	// MessageQueueStopNpcConversation::MessageQueueStopNpcConversation(const MessageQueueStopNpcConversation &)
	
//-----------------------------------------------------------------------
	
inline MessageQueueStopNpcConversation::MessageQueueStopNpcConversation(NetworkId const & npc, StringId const & finalMessage, Unicode::String const & finalProsePackage, Unicode::String const & finalResponse) :
MessageQueue::Data(),
m_npc(npc),
m_finalMessageId(finalMessage),
m_finalMessageProse(finalProsePackage),
m_finalResponse(finalResponse)
{
}

//-----------------------------------------------------------------------

inline MessageQueueStopNpcConversation & MessageQueueStopNpcConversation::operator= (const MessageQueueStopNpcConversation & source)
{
	m_npc = source.m_npc;
	m_finalMessageId = source.m_finalMessageId;
	m_finalMessageProse = source.m_finalMessageProse;
	m_finalResponse = source.m_finalResponse;
	return *this;
}	// MessageQueueStopNpcConversation::operator=

//-----------------------------------------------------------------------

inline const NetworkId & MessageQueueStopNpcConversation::getNpc() const
{
	return m_npc;
}	// MessageQueueStopNpcConversation::getNpc

//-----------------------------------------------------------------------

inline void MessageQueueStopNpcConversation::setNpc(const NetworkId & npc)
{
	m_npc = npc;
}	// MessageQueueStopNpcConversation::setNpc

//-----------------------------------------------------------------------

inline void MessageQueueStopNpcConversation::setFinalMessageId(StringId const & finalMessage)
{
	m_finalMessageId = finalMessage;
}

//-----------------------------------------------------------------------

inline StringId const & MessageQueueStopNpcConversation::getFinalMessageId() const
{
	return m_finalMessageId;
}

//-----------------------------------------------------------------------

inline void MessageQueueStopNpcConversation::setFinalMessageProse(Unicode::String const & finalMessage)
{
	m_finalMessageProse = finalMessage;
}

//-----------------------------------------------------------------------

inline Unicode::String const & MessageQueueStopNpcConversation::getFinalMessageProse() const
{
	return m_finalMessageProse;
}

//-----------------------------------------------------------------------

inline void MessageQueueStopNpcConversation::setFinalResponse(Unicode::String const & finalResponse)
{
	m_finalResponse = finalResponse;
}

//-----------------------------------------------------------------------

inline Unicode::String const & MessageQueueStopNpcConversation::getFinalResponse() const
{
	return m_finalResponse;
}

//-----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueStopNpcConversation_H
