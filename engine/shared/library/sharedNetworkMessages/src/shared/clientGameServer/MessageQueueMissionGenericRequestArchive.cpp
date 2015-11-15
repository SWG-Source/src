//======================================================================
//
// MessageQueueMissionGenericRequestArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericRequestArchive.h"

#include "sharedNetworkMessages/MessageQueueMissionGenericRequest.h"

//======================================================================

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionGenericRequest & target)
	{
		NetworkId missionObjectId;
		NetworkId terminalId;
		uint8 sequenceId = 0;

		get (source, missionObjectId);
		get (source, terminalId);
		get (source, sequenceId);

		target.set (missionObjectId, terminalId, sequenceId);
	}


	void put (ByteStream & target, const MessageQueueMissionGenericRequest & source)
	{
		put (target, source.getMissionObjectId ());
		put (target, source.getTerminalId ());
		put (target, source.getSequenceId ());
	}

}
//======================================================================
