//========================================================================
//
// MessageQueueChangeRoleIconChoice.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueChangeRoleIconChoice.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueChangeRoleIconChoice, CM_changeRoleIconChoice);

//----------------------------------------------------------------------

MessageQueueChangeRoleIconChoice::MessageQueueChangeRoleIconChoice (int newRoleIconChoice, uint8 sequenceId) :
m_roleIconChoice  (newRoleIconChoice),
m_sequenceId      (sequenceId)
{
}

//----------------------------------------------------------------------

/**
 * Class destructor.
 */
MessageQueueChangeRoleIconChoice::~MessageQueueChangeRoleIconChoice ()
{
}

//----------------------------------------------------------------------

void MessageQueueChangeRoleIconChoice::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueChangeRoleIconChoice* const msg = safe_cast<const MessageQueueChangeRoleIconChoice*> (data);
	if (msg)
	{
		Archive::put(target, msg->getRoleIconChoice ());
		Archive::put(target, msg->getSequenceId ());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueChangeRoleIconChoice::unpack (Archive::ReadIterator & source)
{
	int newRoleIconChoice = 0;
	uint8 sequenceId = 0;
	
	Archive::get(source, newRoleIconChoice);
	Archive::get(source, sequenceId);
	
	return new MessageQueueChangeRoleIconChoice(newRoleIconChoice, sequenceId);
}

//----------------------------------------------------------------------

