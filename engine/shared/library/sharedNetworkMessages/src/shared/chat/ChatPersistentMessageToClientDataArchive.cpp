//======================================================================
//
// ChatPersistentMessageToClientDataArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientDataArchive.h"
#include "sharedNetworkMessages/ChatPersistentMessageToClientData.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatPersistentMessageToClientData & target)
	{
		get (source, target.fromCharacterName);
		get (source, target.fromGameCode);
		get (source, target.fromServerCode);
		get (source, target.id);
		get (source, target.isHeader);
		get (source, target.message);
		get (source, target.subject);
		get (source, target.outOfBand);
		get (source, target.status);
		get (source, target.timeStamp);		
	}

	void put (ByteStream & target, const ChatPersistentMessageToClientData & source)
	{
		put (target, source.fromCharacterName);
		put (target, source.fromGameCode);
		put (target, source.fromServerCode);
		put (target, source.id);
		put (target, source.isHeader);
		put (target, source.message);
		put (target, source.subject);
		put (target, source.outOfBand);
		put (target, source.status);
		put (target, source.timeStamp);		
	}
}

//======================================================================
