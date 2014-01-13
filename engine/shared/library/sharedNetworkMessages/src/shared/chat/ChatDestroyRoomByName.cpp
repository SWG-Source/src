// ChatDestroyRoomByName.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatDestroyRoomByName.h"

//-----------------------------------------------------------------------

ChatDestroyRoomByName::ChatDestroyRoomByName(const std::string & r) :
GameNetworkMessage("ChatDestroyRoomByName"),
roomPath(r)
{
	addVariable(roomPath);
}

//-----------------------------------------------------------------------

ChatDestroyRoomByName::ChatDestroyRoomByName(Archive::ReadIterator & source) :
GameNetworkMessage("ChatDestroyRoomByName"),
roomPath()
{
	addVariable(roomPath);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatDestroyRoomByName::~ChatDestroyRoomByName()
{
}

//-----------------------------------------------------------------------

