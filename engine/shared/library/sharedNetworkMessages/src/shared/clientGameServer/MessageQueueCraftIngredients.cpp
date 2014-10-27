//========================================================================
//
// MessageQueueCraftIngredients.cpp
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueCraftIngredients.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

//===================================================================

CONTROLLER_MESSAGE_IMPLEMENTATION(MessageQueueCraftIngredients, CM_ingredientsHopperMessage);

//===================================================================



/**
 * Class destructor.
 */
MessageQueueCraftIngredients::~MessageQueueCraftIngredients()
{
}	// MessageQueueCraftIngredients::~MessageQueueCraftIngredients

void MessageQueueCraftIngredients::pack(const MessageQueue::Data* const data, Archive::ByteStream & target)
{
	const MessageQueueCraftIngredients* const msg = safe_cast<const MessageQueueCraftIngredients*> (data);
	if (msg)
	{
		int count = msg->getIngredientCount();
		Archive::put (target, count);
		for (int i = 0; i < count; ++i)
		{
			const MessageQueueCraftIngredients::Ingredient & ingredient = msg->getIngredient(i);
			uint8 type = static_cast<uint8>(ingredient.type);
			Archive::put (target, ingredient.name);
			Archive::put (target, type);
			Archive::put (target, ingredient.quantity);
		}
	}
}

//-----------------------------------------------------------------------


MessageQueue::Data* MessageQueueCraftIngredients::unpack(Archive::ReadIterator & source)
{
	MessageQueueCraftIngredients* const msg = new MessageQueueCraftIngredients();
	
	int count;
	Unicode::String name;
	uint8 type;
	int quantity;
	
	Archive::get(source, count);
	for (int i = 0; i < count; ++i)
	{
		Archive::get(source, name);
		Archive::get(source, type);
		Archive::get(source, quantity);
		msg->addIngredient(name, static_cast<MessageQueueCraftIngredients::IngredientType>(type), quantity);
	}
	return msg;
}

