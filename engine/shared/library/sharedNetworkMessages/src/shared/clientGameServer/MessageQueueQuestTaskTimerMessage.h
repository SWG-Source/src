//======================================================================
//
// MessageQueueQuestTaskTimerMessage.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueQuestTaskTimerMessage_H
#define INCLUDED_MessageQueueQuestTaskTimerMessage_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

/**
*
*/

class MessageQueueQuestTaskTimerMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueQuestTaskTimerMessage (std::string const & questName, int taskId, Unicode::String const & sourceName, int timerLength);
	virtual                ~MessageQueueQuestTaskTimerMessage();

	std::string const & getQuestName() const;
	int getTaskId() const;
	Unicode::String const & getSourceName() const;
	int getTimerLength() const;

private:
	//disabled
	MessageQueueQuestTaskTimerMessage&  operator= (const MessageQueueQuestTaskTimerMessage & source);
	MessageQueueQuestTaskTimerMessage(const MessageQueueQuestTaskTimerMessage & source);

private:
	std::string m_questName;
	int m_taskId;
	Unicode::String m_sourceName;
	int m_timerLength;
};

//-----------------------------------------------------------------------

inline std::string const & MessageQueueQuestTaskTimerMessage::getQuestName() const
{
	return m_questName;
}

//-----------------------------------------------------------------------

inline int MessageQueueQuestTaskTimerMessage::getTaskId() const
{
	return m_taskId;
}

//-----------------------------------------------------------------------

inline Unicode::String const & MessageQueueQuestTaskTimerMessage::getSourceName() const
{
	return m_sourceName;
}

//-----------------------------------------------------------------------

inline int MessageQueueQuestTaskTimerMessage::getTimerLength() const
{
	return m_timerLength;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueQuestTaskTimerMessage_H

