//======================================================================
//
// MessageQueueDraftSlotsDataOptionArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsDataOptionArchive.h"

#include "sharedNetworkMessages/MessageQueueDraftSlotsDataOption.h"
#include "Archive/Archive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get (ReadIterator & source, MessageQueueDraftSlotsDataOption & target)
	{
		Archive::get(source, target.name);
		Archive::get(source, target.ingredient);

		uint8 type = 0;
		Archive::get(source, type);
		target.type = static_cast<Crafting::IngredientType>(type);

		Archive::get(source, target.amountNeeded);
	}
	
	//----------------------------------------------------------------------

	void put (ByteStream & target, const MessageQueueDraftSlotsDataOption & source)
	{
		Archive::put(target, source.name);
		Archive::put(target, source.ingredient);

		uint8 type = static_cast<uint8>(source.type);
		Archive::put(target, type);

		Archive::put(target, source.amountNeeded);
	}
}

//======================================================================
