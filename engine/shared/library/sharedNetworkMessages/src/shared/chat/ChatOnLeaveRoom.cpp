// ChatOnLeaveRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnLeaveRoom.h"

//-----------------------------------------------------------------------

ChatOnLeaveRoom::ChatOnLeaveRoom(const unsigned int s, const unsigned int r, const unsigned int n, const ChatAvatarId & c) :
GameNetworkMessage("ChatOnLeaveRoom"),
characterName(c),
resultCode(r),
roomId(n),
sequence(s)
{
	addVariable(characterName);
	addVariable(resultCode);
	addVariable(roomId);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnLeaveRoom::ChatOnLeaveRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnLeaveRoom"),
characterName(),
resultCode(),
roomId(),
sequence()
{
	addVariable(characterName);
	addVariable(resultCode);
	addVariable(roomId);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnLeaveRoom::~ChatOnLeaveRoom()
{
}

//-----------------------------------------------------------------------

