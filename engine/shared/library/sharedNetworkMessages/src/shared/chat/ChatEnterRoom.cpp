// ChatEnterRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatEnterRoom.h"

//-----------------------------------------------------------------------

ChatEnterRoom::ChatEnterRoom(const unsigned int s, const std::string & n) :
GameNetworkMessage("ChatEnterRoom"),
roomName(n),
sequence(s)
{
	addVariable(sequence);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatEnterRoom::ChatEnterRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatEnterRoom"),
roomName(),
sequence()
{
	addVariable(sequence);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatEnterRoom::~ChatEnterRoom()
{
}

//-----------------------------------------------------------------------

