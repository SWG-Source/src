// ChatOnSendInstantMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnSendInstantMessage.h"

//-----------------------------------------------------------------------

ChatOnSendInstantMessage::ChatOnSendInstantMessage(const unsigned int s, const unsigned int r) :
GameNetworkMessage("ChatOnSendInstantMessage"),
result(r),
sequence(s)
{
	addVariable(result);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnSendInstantMessage::ChatOnSendInstantMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnSendInstantMessage"),
result(),
sequence()
{
	addVariable(result);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnSendInstantMessage::~ChatOnSendInstantMessage()
{
}

//-----------------------------------------------------------------------

