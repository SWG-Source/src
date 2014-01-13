// ChatRoomMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatRoomMessage.h"

//-----------------------------------------------------------------------

ChatRoomMessage::ChatRoomMessage(const unsigned int r, const ChatAvatarId & f, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatRoomMessage"),
fromName(f),
fromRoom(r),
message(m),
outOfBand(o)
{
	addVariable(fromName);
	addVariable(fromRoom);
	addVariable(message);
	addVariable(outOfBand);
}

//-----------------------------------------------------------------------

ChatRoomMessage::ChatRoomMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRoomMessage"),
fromName(),
message(),
outOfBand()
{
	addVariable(fromName);
	addVariable(fromRoom);
	addVariable(message);
	addVariable(outOfBand);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRoomMessage::~ChatRoomMessage()
{
}

//-----------------------------------------------------------------------

