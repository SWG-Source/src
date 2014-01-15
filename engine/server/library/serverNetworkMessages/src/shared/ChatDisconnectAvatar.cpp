// ChatDisconnectAvatar.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "ChatDisconnectAvatar.h"

//-----------------------------------------------------------------------

ChatDisconnectAvatar::ChatDisconnectAvatar(const NetworkId & i) :
GameNetworkMessage("ChatDisconnectAvatar"),
characterId(i)
{
	addVariable(characterId);
}

//-----------------------------------------------------------------------

ChatDisconnectAvatar::ChatDisconnectAvatar(Archive::ReadIterator & source) :
GameNetworkMessage("ChatDisconnectAvatar"),
characterId()
{
	addVariable(characterId);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatDisconnectAvatar::~ChatDisconnectAvatar()
{
}

//-----------------------------------------------------------------------

