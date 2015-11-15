// ChatOnSendRoomInvitation.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnSendRoomInvitation.h"

//-----------------------------------------------------------------------

ChatOnSendRoomInvitation::ChatOnSendRoomInvitation(const unsigned int s, const unsigned int r) :
GameNetworkMessage("ChatOnSendRoomInvitation"),
result(r),
sequence(s)
{
	addVariable(result);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnSendRoomInvitation::ChatOnSendRoomInvitation(Archive::ReadIterator & source) : 
GameNetworkMessage("ChatOnSendRoomInvitation"),
result(),
sequence()
{
	addVariable(result);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnSendRoomInvitation::~ChatOnSendRoomInvitation()
{
}

//-----------------------------------------------------------------------

