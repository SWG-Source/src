// ChatOnEnteredRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnEnteredRoom.h"

//-----------------------------------------------------------------------

ChatOnEnteredRoom::ChatOnEnteredRoom(const unsigned int s, const unsigned int r, const unsigned int n, const ChatAvatarId & c) :
GameNetworkMessage("ChatOnEnteredRoom"),
characterName(c),
result(r),
roomId(n),
sequence(s)
{
	addVariable(characterName);
	addVariable(result);
	addVariable(roomId);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnEnteredRoom::ChatOnEnteredRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnEnteredRoom"),
characterName(),
result(),
roomId(),
sequence()
{
	addVariable(characterName);
	addVariable(result);
	addVariable(roomId);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnEnteredRoom::~ChatOnEnteredRoom()
{
}

//-----------------------------------------------------------------------

