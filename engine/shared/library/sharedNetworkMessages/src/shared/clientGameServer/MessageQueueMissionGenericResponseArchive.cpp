//======================================================================
//
// MessageQueueMissionGenericResponseArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponseArchive.h"

#include "sharedNetworkMessages/MessageQueueMissionGenericResponse.h"

//======================================================================

namespace Archive
{
	void get (ReadIterator & source, MessageQueueMissionGenericResponse & target)
	{
		NetworkId id;
		bool  success = false;
		uint8 sequenceId = 0;

		get (source, id);
		get (source, success);
		get (source, sequenceId);

		target.set (id, success, sequenceId);
	}


	void put (ByteStream & target, const MessageQueueMissionGenericResponse & source)
	{
		put (target, source.getMissionObjectId ());
		put (target, source.getSuccess ());
		put (target, source.getSequenceId ());
	}

}
//======================================================================
