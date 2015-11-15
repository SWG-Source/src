// ChatServerStatus.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatServerStatus.h"

//-----------------------------------------------------------------------

ChatServerStatus::ChatServerStatus(const bool s) :
GameNetworkMessage("ChatServerStatus"),
status(s)
{
	addVariable(status);
}

//-----------------------------------------------------------------------

ChatServerStatus::ChatServerStatus(Archive::ReadIterator & source) :
GameNetworkMessage("ChatServerStatus"),
status()
{
	addVariable(status);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatServerStatus::~ChatServerStatus()
{
}

//-----------------------------------------------------------------------
