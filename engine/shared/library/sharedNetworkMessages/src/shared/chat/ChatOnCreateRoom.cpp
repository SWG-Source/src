// ChatOnCreateRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnCreateRoom.h"

//-----------------------------------------------------------------------

ChatOnCreateRoom::ChatOnCreateRoom(const unsigned int s, const unsigned int c, const ChatRoomData & r) :
GameNetworkMessage("ChatOnCreateRoom"),
resultCode(c),
roomData(r),
sequence(s)
{
	addVariable(resultCode);
	addVariable(roomData);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnCreateRoom::ChatOnCreateRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnCreateRoom"),
resultCode(),
roomData(),
sequence()
{
	addVariable(resultCode);
	addVariable(roomData);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnCreateRoom::~ChatOnCreateRoom()
{
}

//-----------------------------------------------------------------------

