//========================================================================
//
// MessageQueueStartNpcConversation.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueStartNpcConversation, CM_npcConversationStart);

//===================================================================



/**
 * Class destructor.
 */
MessageQueueStartNpcConversation::~MessageQueueStartNpcConversation()
{
}	// MessageQueueStartNpcConversation::~MessageQueueStartNpcConversation

void MessageQueueStartNpcConversation::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueStartNpcConversation* const msg = safe_cast<const MessageQueueStartNpcConversation*> (data);
	
	if (msg)
	{
		Archive::put(target, msg->getNpc());
		uint8 temp8 = static_cast<uint8>(msg->getStarter());
		Archive::put(target, temp8);
		Archive::put(target, msg->getConversationName());
		Archive::put(target, msg->getAppearanceOverrideTemplateCrc());
	}
}	// ControllerMessageFactory::packStartNpcConversation

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueStartNpcConversation::unpack(Archive::ReadIterator & source)
{
	MessageQueueStartNpcConversation * msg = new MessageQueueStartNpcConversation;
	
	NetworkId npc;
	Archive::get(source, npc);
	msg->setNpc(npc);

	uint8 starter;
	Archive::get(source, starter);
	msg->setStarter(static_cast<NpcConversationData::ConversationStarter>(starter));
	
	std::string convo;
	Archive::get(source, convo);
	msg->setConversationName(convo);

	uint32 appearanceOverrideCrc;
	Archive::get(source, appearanceOverrideCrc);
	msg->setAppearanceOverrideTemplateCrc(appearanceOverrideCrc);

	return msg;
}	// ControllerMessageFactory::unpackStartNpcConversation

//-----------------------------------------------------------------------

