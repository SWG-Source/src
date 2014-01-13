// ServerInfoArchive.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ServerInfo.h"
#include "sharedNetworkMessages/ServerInfoArchive.h"

//-----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ServerInfo & target)
	{
		get (source, target.ipAddress);	
		get (source, target.serverId);	
		get (source, target.systemPid);
		get (source, target.sceneId);
	}

	void put (ByteStream & target, const ServerInfo & source)
	{
		put (target, source.ipAddress);
		put (target, source.serverId);
		put (target, source.systemPid);
		put (target, source.sceneId);
	}
}

//-----------------------------------------------------------------------

