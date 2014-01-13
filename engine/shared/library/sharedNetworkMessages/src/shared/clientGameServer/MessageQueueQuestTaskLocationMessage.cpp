//======================================================================
//
// MessageQueueQuestTaskLocationMessage.cpp
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskLocationMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueQuestTaskLocationMessage, CM_questTaskLocationData);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueQuestTaskLocationMessage::MessageQueueQuestTaskLocationMessage (std::string const & questName, int taskId, Location const & location)
 : Data ()
 , m_questName(questName)
 , m_taskId(taskId)
 , m_location(location)
{
}

//-----------------------------------------------------------------------

MessageQueueQuestTaskLocationMessage::~MessageQueueQuestTaskLocationMessage()
{
}

//-----------------------------------------------------------------------

void MessageQueueQuestTaskLocationMessage::pack(MessageQueue::Data const * const data, Archive::ByteStream & target)
{
	const MessageQueueQuestTaskLocationMessage* const msg = safe_cast<const MessageQueueQuestTaskLocationMessage*>(data);
	if (msg)
	{
		Archive::put(target, msg->getQuestName());
		Archive::put(target, msg->getTaskId());
		Archive::put(target, msg->getLocation());
	}
}

//----------------------------------------------------------------------

MessageQueue::Data * MessageQueueQuestTaskLocationMessage::unpack(Archive::ReadIterator & source)
{
	std::string questName;
	Archive::get(source, questName);

	int taskId = 0;
	Archive::get(source, taskId);

	Location location;
	Archive::get(source, location);

	MessageQueue::Data * result = new MessageQueueQuestTaskLocationMessage(questName, taskId, location);

	return result;
}

//----------------------------------------------------------------------
