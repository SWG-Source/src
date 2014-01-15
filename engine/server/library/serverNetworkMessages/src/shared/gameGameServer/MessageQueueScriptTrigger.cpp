// MessageQueueScriptTrigger.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageQueueScriptTrigger.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueScriptTrigger, CM_scriptTrigger);

//===================================================================


//-----------------------------------------------------------------------

MessageQueueScriptTrigger::MessageQueueScriptTrigger(const int i, const Archive::ByteStream & d) :
scriptParamData(d),
triggerId(i)
{
}

//-----------------------------------------------------------------------

MessageQueueScriptTrigger::~MessageQueueScriptTrigger()
{
}

//-----------------------------------------------------------------------


void MessageQueueScriptTrigger::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueScriptTrigger* const msg = safe_cast<const MessageQueueScriptTrigger*> (data);
	if(msg)
	{
		Archive::put(target, msg->getTriggerId());
		Archive::put(target, msg->getScriptParamData());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* MessageQueueScriptTrigger::unpack(Archive::ReadIterator & source)
{
	Archive::ByteStream paramData;
	int triggerId;
	Archive::get(source, triggerId);
	Archive::get(source, paramData);
	return new MessageQueueScriptTrigger(triggerId, paramData);
}

//-----------------------------------------------------------------------
