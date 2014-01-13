// ChatInstantMessageToClient.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatInstantMessageToClient.h"

//-----------------------------------------------------------------------

ChatInstantMessageToClient::ChatInstantMessageToClient(const ChatAvatarId & f, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatInstantMessageToClient"),
fromName(f),
message(m),
outOfBand(o)
{
	addVariable(fromName);
	addVariable(message);
	addVariable(outOfBand);
}

//-----------------------------------------------------------------------

ChatInstantMessageToClient::ChatInstantMessageToClient(Archive::ReadIterator & source) :
GameNetworkMessage("ChatInstantMessageToClient"),
fromName(),
message(),
outOfBand()
{
	addVariable(fromName);
	addVariable(message);
	addVariable(outOfBand);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatInstantMessageToClient::~ChatInstantMessageToClient()
{
}

//-----------------------------------------------------------------------

