//========================================================================
//
// MessageQueueCraftCustomization.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftCustomization.h"

#include "sharedObject/Object.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(MessageQueueCraftCustomization, CM_setCustomizationData);

//===================================================================


/**
 * Class destructor.
 */
MessageQueueCraftCustomization::~MessageQueueCraftCustomization()
{
}	


void MessageQueueCraftCustomization::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftCustomization* const msg = safe_cast<const MessageQueueCraftCustomization*> (data);
	if (msg)
	{
		Archive::put(target, msg->getName());
		int8 temp8 = static_cast<int8>(msg->getAppearance());
		Archive::put(target, temp8);
		Archive::put(target, msg->getItemCount());
		int count = msg->getCustomization().size();
		temp8 = static_cast<int8>(count);
		Archive::put(target, temp8);
		for (int i = 0; i < count; ++i)
		{
			const Crafting::CustomValue & custom = msg->getCustomization()[i];
			Archive::put(target, custom.property);
			Archive::put(target, custom.value);
		}
	}
}

//----------------------------------------------------------------------

MessageQueue::Data* MessageQueueCraftCustomization::unpack(Archive::ReadIterator & source)
{
int8 temp8;
Unicode::String name;
int property, value, itemCount;

	UNREF(source);

	MessageQueueCraftCustomization * msg = new MessageQueueCraftCustomization();
	
	Archive::get(source, name);
	msg->setName(name);
	
	Archive::get(source, temp8);
	msg->setAppearance(temp8);

	Archive::get(source, itemCount);
	msg->setItemCount(itemCount);

	Archive::get(source, temp8);
	int count = temp8;
	for (int i = 0; i < count; ++i)
	{
		Archive::get(source, property);
		Archive::get(source, value);
		msg->addCustomization(property, value);
	}

	return msg;
}

//----------------------------------------------------------------------
