//======================================================================
//
// MessageQueueDraftSlotsDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsDataArchive.h"

#include "sharedNetworkMessages/MessageQueueDraftSlotsData.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsDataOptionArchive.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsDataOption.h"
#include "Archive/Archive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get (ReadIterator & source, MessageQueueDraftSlotsData & target)
	{
		Archive::get(source, target.name);
		Archive::get(source, target.optional);

		int optionCount = 0;
		Archive::get(source, optionCount);

		MessageQueueDraftSlotsDataOption option;
		bool componentSlot = false;

		for (int j = 0; j < optionCount; ++j)
		{
			Archive::get (source, option);
			if (option.type == Crafting::IT_item ||
				option.type == Crafting::IT_template)
			{
				componentSlot = true;
			}

			target.options.push_back (option);
		}

		if (componentSlot)
			Archive::get(source, target.hardpoint);	
	}
	
	//----------------------------------------------------------------------

	void put (ByteStream & target, const MessageQueueDraftSlotsData & source)
	{
		Archive::put (target, source.name);
		Archive::put (target, source.optional);

		int optionCount = source.options.size();
		Archive::put (target, optionCount);

		bool componentSlot = false;

		for (int j = 0; j < optionCount; ++j)
		{
			const MessageQueueDraftSlotsDataOption & option = source.options[j];

			Archive::put (target, option);

			if (option.type == Crafting::IT_item ||
				option.type == Crafting::IT_template)
			{
				componentSlot = true;
			}
		}

		if (componentSlot)
			Archive::put(target, source.hardpoint);
	}
}

//======================================================================
