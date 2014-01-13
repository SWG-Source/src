// ChatRequestPersistentMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatRequestPersistentMessage.h"

//-----------------------------------------------------------------------

ChatRequestPersistentMessage::ChatRequestPersistentMessage(const unsigned int s, const unsigned int i) :
GameNetworkMessage("ChatRequestPersistentMessage"),
messageId(i),
sequence(s)
{
	addVariable(sequence);
	addVariable(messageId);
}

//-----------------------------------------------------------------------

ChatRequestPersistentMessage::ChatRequestPersistentMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRequestPersistentMessage"),
messageId()
{
	addVariable(sequence);
	addVariable(messageId);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRequestPersistentMessage::~ChatRequestPersistentMessage()
{
}

//-----------------------------------------------------------------------

