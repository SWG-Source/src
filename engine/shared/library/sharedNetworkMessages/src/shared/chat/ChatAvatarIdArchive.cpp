//======================================================================
//
// ChatAvatarIdArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

#include "sharedNetworkMessages/ChatAvatarId.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ChatAvatarId & target)
	{
		get (source, target.gameCode);	
		get (source, target.cluster);	
		get (source, target.name);
	}

	void put (ByteStream & target, const ChatAvatarId & source)
	{
		put (target, source.gameCode);
		put (target, source.cluster);
		put (target, source.name);
	}
}

//======================================================================
