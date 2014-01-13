//======================================================================
//
// MessageQueueQuestTaskTimerMessage.cpp
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskTimerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueQuestTaskTimerMessage, CM_questTaskTimerData);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueQuestTaskTimerMessage::MessageQueueQuestTaskTimerMessage (std::string const & questName, int taskId, Unicode::String const & sourceName, int timerLength) :
Data (),
m_questName(questName),
m_taskId(taskId),
m_sourceName(sourceName),
m_timerLength(timerLength)
{
}

//-----------------------------------------------------------------------

MessageQueueQuestTaskTimerMessage::~MessageQueueQuestTaskTimerMessage()
{
}

//-----------------------------------------------------------------------

void MessageQueueQuestTaskTimerMessage::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueQuestTaskTimerMessage* const msg = safe_cast<const MessageQueueQuestTaskTimerMessage*> (data);
	if (msg)
	{
		Archive::put(target, msg->getQuestName());
		Archive::put(target, msg->getTaskId());
		Archive::put(target, msg->getSourceName());
		Archive::put(target, msg->getTimerLength());
	}
}

//----------------------------------------------------------------------


MessageQueue::Data * MessageQueueQuestTaskTimerMessage::unpack(Archive::ReadIterator & source)
{
	std::string questName;
	Archive::get(source, questName);

	int taskId = 0;
	Archive::get(source, taskId);

	Unicode::String sourceName;
	Archive::get(source, sourceName);

	int timerLength = 0;
	Archive::get(source, timerLength);

	MessageQueue::Data * result = new MessageQueueQuestTaskTimerMessage (questName, taskId, sourceName, timerLength);
	return result;
}

//----------------------------------------------------------------------

