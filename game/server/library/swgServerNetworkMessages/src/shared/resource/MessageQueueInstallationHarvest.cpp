// ======================================================================
//
// MessageQueueInstallationHarvest.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "MessageQueueInstallationHarvest.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

// ======================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueInstallationHarvest, CM_installationHarvest);

// ======================================================================

MessageQueueInstallationHarvest::MessageQueueInstallationHarvest() 
{
}	

// ----------------------------------------------------------------------

MessageQueueInstallationHarvest::~MessageQueueInstallationHarvest()
{
	
}

// ----------------------------------------------------------------------

void MessageQueueInstallationHarvest::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	UNREF(data);
	UNREF(target);
//	const MessageQueueInstallationHarvest* const msg = safe_cast<const MessageQueueInstallationHarvest*> (data);
}

// ----------------------------------------------------------------------

MessageQueue::Data* MessageQueueInstallationHarvest::unpack(Archive::ReadIterator & source)
{
	UNREF(source);
	return new MessageQueueInstallationHarvest();
}

// ======================================================================
