// ChatQueryRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatQueryRoom.h"

//-----------------------------------------------------------------------

ChatQueryRoom::ChatQueryRoom(const unsigned int s, const std::string & n) :
GameNetworkMessage("ChatQueryRoom"),
roomName(n),
sequence(s)
{
	addVariable(sequence);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatQueryRoom::ChatQueryRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatQueryRoom"),
roomName(),
sequence()
{
	addVariable(sequence);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatQueryRoom::~ChatQueryRoom()
{
}

//-----------------------------------------------------------------------

