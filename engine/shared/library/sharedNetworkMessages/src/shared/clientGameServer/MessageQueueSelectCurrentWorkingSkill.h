//========================================================================
//
// MessageQueueSelectCurrentWorkingSkill.h
//
// copyright 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueSelectCurrentWorkingSkill_H
#define INCLUDED_MessageQueueSelectCurrentWorkingSkill_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueSelectCurrentWorkingSkill : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueSelectCurrentWorkingSkill(std::string const & currentWorkingSkill);
	virtual ~MessageQueueSelectCurrentWorkingSkill();

	std::string const & getCurrentWorkingSkill() const;

private:

	std::string m_currentWorkingSkill;

private:
	MessageQueueSelectCurrentWorkingSkill();
	MessageQueueSelectCurrentWorkingSkill(const MessageQueueSelectCurrentWorkingSkill &);
	MessageQueueSelectCurrentWorkingSkill & operator=(const MessageQueueSelectCurrentWorkingSkill &);
};

//----------------------------------------------------------------------

inline std::string const & MessageQueueSelectCurrentWorkingSkill::getCurrentWorkingSkill() const
{
	return m_currentWorkingSkill;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueSelectCurrentWorkingSkill_H
