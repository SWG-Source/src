//======================================================================
//
// MessageQueueQuestTaskCounterMessage.cpp
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskCounterMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueQuestTaskCounterMessage, CM_questTaskCounterData);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueQuestTaskCounterMessage::MessageQueueQuestTaskCounterMessage (std::string const & questName, int taskId, Unicode::String const & sourceName, int counter, int counterMax) :
Data (),
m_questName(questName),
m_taskId(taskId),
m_sourceName(sourceName),
m_counterValue(counter),
m_counterMax(counterMax)
{
}

//-----------------------------------------------------------------------

MessageQueueQuestTaskCounterMessage::~MessageQueueQuestTaskCounterMessage()
{
}

//-----------------------------------------------------------------------

void MessageQueueQuestTaskCounterMessage::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueQuestTaskCounterMessage* const msg = safe_cast<const MessageQueueQuestTaskCounterMessage*> (data);
	if (msg)
	{
		Archive::put(target, msg->getQuestName());
		Archive::put(target, msg->getTaskId());
		Archive::put(target, msg->getSourceName());
		Archive::put(target, msg->getCounter());
		Archive::put(target, msg->getCounterMax());
	}
}

//----------------------------------------------------------------------


MessageQueue::Data * MessageQueueQuestTaskCounterMessage::unpack(Archive::ReadIterator & source)
{
	std::string questName;
	Archive::get(source, questName);

	int taskId = 0;
	Archive::get(source, taskId);

	Unicode::String sourceName;
	Archive::get(source, sourceName);

	int counter = 0;
	Archive::get(source, counter);

	int counterMax = 0;
	Archive::get(source, counterMax);

	MessageQueue::Data * result = new MessageQueueQuestTaskCounterMessage (questName, taskId, sourceName, counter, counterMax);
	return result;
}

//----------------------------------------------------------------------

