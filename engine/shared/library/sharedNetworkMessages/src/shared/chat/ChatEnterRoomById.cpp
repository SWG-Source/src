// ChatEnterRoomById.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatEnterRoomById.h"

//-----------------------------------------------------------------------

ChatEnterRoomById::ChatEnterRoomById(const unsigned int s, const unsigned int n, const std::string & name) :
GameNetworkMessage("ChatEnterRoomById"),
roomId(n),
roomName(name),
sequence(s)
{
	addVariable(sequence);
	addVariable(roomId);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatEnterRoomById::ChatEnterRoomById(Archive::ReadIterator & source) :
GameNetworkMessage("ChatEnterRoomById"),
roomId(),
roomName(),
sequence()
{
	addVariable(sequence);
	addVariable(roomId);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatEnterRoomById::~ChatEnterRoomById()
{
}

//-----------------------------------------------------------------------

