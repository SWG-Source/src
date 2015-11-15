//========================================================================
//
// MessageQueueSelectCurrentWorkingSkill.cpp
//
// copyright 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSelectCurrentWorkingSkill.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueSelectCurrentWorkingSkill, CM_setCurrentWorkingSkill);

//----------------------------------------------------------------------

MessageQueueSelectCurrentWorkingSkill::MessageQueueSelectCurrentWorkingSkill(std::string const & currentWorkingSkill) :
m_currentWorkingSkill(currentWorkingSkill)
{
}

//----------------------------------------------------------------------

MessageQueueSelectCurrentWorkingSkill::~MessageQueueSelectCurrentWorkingSkill()
{
}

//----------------------------------------------------------------------

void MessageQueueSelectCurrentWorkingSkill::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	MessageQueueSelectCurrentWorkingSkill const * const msg = safe_cast<const MessageQueueSelectCurrentWorkingSkill*>(data);
	if (msg)
	{
		Archive::put(target, msg->getCurrentWorkingSkill());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * MessageQueueSelectCurrentWorkingSkill::unpack(Archive::ReadIterator & source)
{
	std::string currentWorkingSkill;

	Archive::get(source, currentWorkingSkill);
	
	return new MessageQueueSelectCurrentWorkingSkill(currentWorkingSkill);
}

//----------------------------------------------------------------------

