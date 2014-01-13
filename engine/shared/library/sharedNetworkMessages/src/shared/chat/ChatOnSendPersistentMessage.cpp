// ChatOnSendPersistentMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnSendPersistentMessage.h"

//-----------------------------------------------------------------------

ChatOnSendPersistentMessage::ChatOnSendPersistentMessage(const unsigned int s, const unsigned int r) :
GameNetworkMessage("ChatOnSendPersistentMessage"),
result(r),
sequence(s)
{
	addVariable(result);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnSendPersistentMessage::ChatOnSendPersistentMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnSendPersistentMessage"),
result(),
sequence()
{
	addVariable(result);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnSendPersistentMessage::~ChatOnSendPersistentMessage()
{
}

//-----------------------------------------------------------------------

