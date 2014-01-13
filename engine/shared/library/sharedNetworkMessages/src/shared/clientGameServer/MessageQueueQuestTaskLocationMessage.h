//======================================================================
//
// MessageQueueQuestTaskLocationMessage.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueQuestTaskLocationMessage_H
#define INCLUDED_MessageQueueQuestTaskLocationMessage_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"
#include "sharedUtility/Location.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

/**
*
*/

class MessageQueueQuestTaskLocationMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueQuestTaskLocationMessage(std::string const & questName, int taskId, Location const & location);
	virtual ~MessageQueueQuestTaskLocationMessage();

	std::string const & getQuestName() const;
	int getTaskId() const;
	Location const & getLocation() const;

private:
	//disabled
	MessageQueueQuestTaskLocationMessage&  operator= (const MessageQueueQuestTaskLocationMessage & source);
	MessageQueueQuestTaskLocationMessage(const MessageQueueQuestTaskLocationMessage & source);

private:
	std::string m_questName;
	int m_taskId;
	Location m_location;
};

//-----------------------------------------------------------------------

inline std::string const & MessageQueueQuestTaskLocationMessage::getQuestName() const
{
	return m_questName;
}

//-----------------------------------------------------------------------

inline int MessageQueueQuestTaskLocationMessage::getTaskId() const
{
	return m_taskId;
}

//-----------------------------------------------------------------------

inline Location const & MessageQueueQuestTaskLocationMessage::getLocation() const
{
	return m_location;
}

//-----------------------------------------------------------------------

#endif	// _MessageQueueQuestTaskLocationMessage_H
