// ChatOnDestroyRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnDestroyRoom.h"

//-----------------------------------------------------------------------

ChatOnDestroyRoom::ChatOnDestroyRoom(const unsigned int s, const unsigned int c, const unsigned int r, const ChatAvatarId & d) :
GameNetworkMessage("ChatOnDestroyRoom"),
destroyer(d),
resultCode(c),
roomId(r),
sequence(s)
{
	addVariable(destroyer);
	addVariable(resultCode);
	addVariable(roomId);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnDestroyRoom::ChatOnDestroyRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnDestroyRoom"),
destroyer(),
resultCode(),
roomId(),
sequence()
{
	addVariable(destroyer);
	addVariable(resultCode);
	addVariable(roomId);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnDestroyRoom::~ChatOnDestroyRoom()
{
}

//-----------------------------------------------------------------------

