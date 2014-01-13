// ChatRequestRoomList.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatRequestRoomList.h"

//-----------------------------------------------------------------------

ChatRequestRoomList::ChatRequestRoomList() :
GameNetworkMessage("ChatRequestRoomList")
{
}

//-----------------------------------------------------------------------

ChatRequestRoomList::ChatRequestRoomList(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRequestRoomList")
{
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRequestRoomList::~ChatRequestRoomList()
{
}

//-----------------------------------------------------------------------

