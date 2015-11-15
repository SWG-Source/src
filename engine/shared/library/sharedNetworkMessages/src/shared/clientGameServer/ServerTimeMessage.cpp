// ServerTimeMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ServerTimeMessage.h"

//-----------------------------------------------------------------------

ServerTimeMessage::ServerTimeMessage(const int64 t) :
GameNetworkMessage("ServerTimeMessage"),
timeSeconds(t)
{
	addVariable(timeSeconds);
}

//-----------------------------------------------------------------------

ServerTimeMessage::ServerTimeMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ServerTimeMessage"),
timeSeconds()
{
	addVariable(timeSeconds);
	unpack(source);
}

//-----------------------------------------------------------------------

ServerTimeMessage::~ServerTimeMessage()
{
}

//-----------------------------------------------------------------------
