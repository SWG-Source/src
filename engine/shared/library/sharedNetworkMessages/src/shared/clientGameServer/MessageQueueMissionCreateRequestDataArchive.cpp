//======================================================================
//
// MessageQueueMissionCreateRequestDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequestDataArchive.h"

#include "sharedNetworkMessages/MessageQueueMissionCreateRequestData.h"

//======================================================================

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionCreateRequestData & target)
	{
		get (source, target.type);
		get (source, target.idAssignee);
		get (source, target.idTerminal);
		get (source, target.idTarget);
		get (source, target.reward);
	}

	void put (ByteStream & target, const MessageQueueMissionCreateRequestData & source)
	{
		put (target, source.type);
		put (target, source.idAssignee);
		put (target, source.idTerminal);
		put (target, source.idTarget);
		put (target, source.reward);
	}
}

//======================================================================
