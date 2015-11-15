// ChatDestroyRoom.cpp
// Copyright 2000-02, Sony Online Destroytainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatDestroyRoom.h"

//-----------------------------------------------------------------------

ChatDestroyRoom::ChatDestroyRoom(const unsigned int s, const unsigned int i) :
GameNetworkMessage("ChatDestroyRoom"),
roomId(i),
sequence(s)
{
	addVariable(roomId);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatDestroyRoom::ChatDestroyRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatDestroyRoom"),
roomId(),
sequence()
{
	addVariable(roomId);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatDestroyRoom::~ChatDestroyRoom()
{
}

//-----------------------------------------------------------------------

