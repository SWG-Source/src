//========================================================================
//
// MessageQueueStartNpcConversation.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueStartNpcConversation_H
#define INCLUDED_MessageQueueStartNpcConversation_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/NpcConversationData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"


class MemoryBlockManager;
class MessageQueueStartNpcConversation : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueStartNpcConversation();
	virtual ~MessageQueueStartNpcConversation();

	const std::string &                      getConversationName() const;
	const NetworkId &                        getNpc() const;
	NpcConversationData::ConversationStarter getStarter() const;
	uint32                                   getAppearanceOverrideTemplateCrc() const;
	
	void                 setConversationName(const std::string & name);
	void                 setNpc(const NetworkId & npc);
	void                 setStarter(NpcConversationData::ConversationStarter starter);
	void                 setAppearanceOverrideTemplateCrc(uint32 crc);

private:
	NetworkId                                m_npc;
	NpcConversationData::ConversationStarter m_starter;
	std::string                              m_conversationName;
	uint32                                   m_appearanceOverrideTemplateCrc;
};


inline MessageQueueStartNpcConversation::MessageQueueStartNpcConversation() :
	MessageQueue::Data(),
	m_npc(),
	m_starter(NpcConversationData::CS_Player),
	m_conversationName(),
	m_appearanceOverrideTemplateCrc(0)
{
}	// MessageQueueStartNpcConversation::MessageQueueStartNpcConversation()

inline const std::string & MessageQueueStartNpcConversation::getConversationName() const
{
	return m_conversationName;
}

inline const NetworkId & MessageQueueStartNpcConversation::getNpc() const
{
	return m_npc;
}	// MessageQueueStartNpcConversation::getNpc

inline NpcConversationData::ConversationStarter MessageQueueStartNpcConversation::getStarter() const
{
	return m_starter;
}

inline uint32 MessageQueueStartNpcConversation::getAppearanceOverrideTemplateCrc() const
{
	return m_appearanceOverrideTemplateCrc;
}

inline void MessageQueueStartNpcConversation::setConversationName(const std::string & c)
{
	m_conversationName = c;
}

inline void MessageQueueStartNpcConversation::setNpc(const NetworkId & npc)
{
	m_npc = npc;
}	// MessageQueueStartNpcConversation::setNpc

inline void MessageQueueStartNpcConversation::setStarter(NpcConversationData::ConversationStarter s)
{
	m_starter = s;
}

inline void MessageQueueStartNpcConversation::setAppearanceOverrideTemplateCrc(uint32 const crc)
{
	m_appearanceOverrideTemplateCrc = crc;
}

#endif	// INCLUDED_MessageQueueStartNpcConversation_H
