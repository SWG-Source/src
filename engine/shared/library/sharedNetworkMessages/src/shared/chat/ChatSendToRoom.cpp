// ChatSendToRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatSendToRoom.h"

//-----------------------------------------------------------------------

ChatSendToRoom::ChatSendToRoom(const unsigned int s, const unsigned int r, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatSendToRoom"),
message(m),
outOfBand(o),
roomId(r),
sequence(s)
{
	addVariable(message);
	addVariable(outOfBand);
	addVariable(roomId);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatSendToRoom::ChatSendToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatSendToRoom"),
message(),
outOfBand(),
roomId()
{
	addVariable(message);
	addVariable(outOfBand);
	addVariable(roomId);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatSendToRoom::~ChatSendToRoom()
{
}

//-----------------------------------------------------------------------
