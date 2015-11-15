// ChatDeletePersistentMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatDeletePersistentMessage.h"

//-----------------------------------------------------------------------

ChatDeletePersistentMessage::ChatDeletePersistentMessage(const unsigned int i) :
GameNetworkMessage("ChatDeletePersistentMessage"),
messageId(i)
{
	addVariable(messageId);
}

//-----------------------------------------------------------------------

ChatDeletePersistentMessage::ChatDeletePersistentMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatDeletePersistentMessage"),
messageId()
{
	addVariable(messageId);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatDeletePersistentMessage::~ChatDeletePersistentMessage()
{
}

//-----------------------------------------------------------------------

