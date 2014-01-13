//========================================================================
//
// MessageQueueCraftSelectSchematic.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftSelectSchematic.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCraftSelectSchematic, CM_selectDraftSchematic);

//===================================================================



/**
 * Class destructor.
 */
MessageQueueCraftSelectSchematic::~MessageQueueCraftSelectSchematic()
{
}	// MessageQueueCraftSelectSchematic::~MessageQueueCraftSelectSchematic


void MessageQueueCraftSelectSchematic::pack (const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftSelectSchematic* const msg = safe_cast<const MessageQueueCraftSelectSchematic*> (data);
	if (msg)
	{
		Archive::put(target, msg->getSchematic());
	}
}

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueCraftSelectSchematic::unpack(Archive::ReadIterator & source)
{
	int schematic;
	
	Archive::get(source, schematic);
	
	return new MessageQueueCraftSelectSchematic(schematic);
}

//----------------------------------------------------------------------

