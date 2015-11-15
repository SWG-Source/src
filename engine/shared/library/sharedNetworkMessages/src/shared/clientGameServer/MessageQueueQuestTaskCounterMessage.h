//======================================================================
//
// MessageQueueQuestTaskCounterMessage.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueQuestTaskCounterMessage_H
#define INCLUDED_MessageQueueQuestTaskCounterMessage_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

/**
*
*/

class MessageQueueQuestTaskCounterMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueQuestTaskCounterMessage (std::string const & questName, int taskId, Unicode::String const & sourceName, int counter, int counterMax);
	virtual                ~MessageQueueQuestTaskCounterMessage();

	std::string const & getQuestName() const;
	int getTaskId() const;
	Unicode::String const & getSourceName() const;
	int getCounter() const;
	int getCounterMax() const;

private:
	//disabled
	MessageQueueQuestTaskCounterMessage&  operator= (const MessageQueueQuestTaskCounterMessage & source);
	MessageQueueQuestTaskCounterMessage(const MessageQueueQuestTaskCounterMessage & source);

private:
	std::string m_questName;
	int m_taskId;
	Unicode::String m_sourceName;
	int m_counterValue;
	int m_counterMax;
};

//-----------------------------------------------------------------------

inline std::string const & MessageQueueQuestTaskCounterMessage::getQuestName() const
{
	return m_questName;
}

//-----------------------------------------------------------------------

inline int MessageQueueQuestTaskCounterMessage::getTaskId() const
{
	return m_taskId;
}

//-----------------------------------------------------------------------

inline Unicode::String const & MessageQueueQuestTaskCounterMessage::getSourceName() const
{
	return m_sourceName;
}

//-----------------------------------------------------------------------

inline int MessageQueueQuestTaskCounterMessage::getCounter() const
{
	return m_counterValue;
}

//-----------------------------------------------------------------------

inline int MessageQueueQuestTaskCounterMessage::getCounterMax() const
{
	return m_counterMax;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueQuestTaskCounterMessage_H

