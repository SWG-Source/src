// ChatRoomList.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatRoomList.h"

//-----------------------------------------------------------------------

ChatRoomList::ChatRoomList(const std::vector<ChatRoomData> & r) :
GameNetworkMessage("ChatRoomList"),
roomData()
{
	roomData.set(r);
	addVariable(roomData);
}

//-----------------------------------------------------------------------

ChatRoomList::ChatRoomList(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRoomList"),
roomData()
{
	addVariable(roomData);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRoomList::~ChatRoomList()
{
}

//-----------------------------------------------------------------------
