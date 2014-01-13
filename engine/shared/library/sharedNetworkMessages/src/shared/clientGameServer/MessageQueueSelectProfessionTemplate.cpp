//========================================================================
//
// MessageQueueSelectProfessionTemplate.cpp
//
// copyright 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueSelectProfessionTemplate, CM_setProfessionTemplate);

//----------------------------------------------------------------------

MessageQueueSelectProfessionTemplate::MessageQueueSelectProfessionTemplate(std::string const & professionTemplate) :
m_professionTemplate(professionTemplate)
{
}

//----------------------------------------------------------------------

MessageQueueSelectProfessionTemplate::~MessageQueueSelectProfessionTemplate()
{
}

//----------------------------------------------------------------------

void MessageQueueSelectProfessionTemplate::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	MessageQueueSelectProfessionTemplate const * const msg = safe_cast<const MessageQueueSelectProfessionTemplate*>(data);
	if (msg)
	{
		Archive::put(target, msg->getProfessionTemplate());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data * MessageQueueSelectProfessionTemplate::unpack(Archive::ReadIterator & source)
{
	std::string professionTemplate;

	Archive::get(source, professionTemplate);
	
	return new MessageQueueSelectProfessionTemplate(professionTemplate);
}

//----------------------------------------------------------------------

