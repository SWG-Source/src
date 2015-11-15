// ChatOnConnectAvatar.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnConnectAvatar.h"

//-----------------------------------------------------------------------

ChatOnConnectAvatar::ChatOnConnectAvatar() :
GameNetworkMessage("ChatOnConnectAvatar")
{
}

//-----------------------------------------------------------------------

ChatOnConnectAvatar::ChatOnConnectAvatar(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnConnectAvatar")
{
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnConnectAvatar::~ChatOnConnectAvatar()
{
}

//-----------------------------------------------------------------------

