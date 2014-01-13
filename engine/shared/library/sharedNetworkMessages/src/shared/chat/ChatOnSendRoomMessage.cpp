// ChatOnSendRoomMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnSendRoomMessage.h"

//-----------------------------------------------------------------------

ChatOnSendRoomMessage::ChatOnSendRoomMessage(const unsigned int s, const unsigned int r) :
GameNetworkMessage("ChatOnSendRoomMessage"),
result(r),
sequence(s)
{
	addVariable(result);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnSendRoomMessage::ChatOnSendRoomMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnSendRoomMessage"),
result(),
sequence()
{
	addVariable(result);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnSendRoomMessage::~ChatOnSendRoomMessage()
{
}

//-----------------------------------------------------------------------

