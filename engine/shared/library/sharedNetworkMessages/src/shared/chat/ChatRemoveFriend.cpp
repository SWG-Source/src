// ChatRemoveFriend.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatRemoveFriend.h"

//-----------------------------------------------------------------------

ChatRemoveFriend::ChatRemoveFriend(const ChatAvatarId & c) :
GameNetworkMessage("ChatRemoveFriend"),
characterName(c)
{
	addVariable(characterName);
}

//-----------------------------------------------------------------------

ChatRemoveFriend::ChatRemoveFriend(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRemoveFriend"),
characterName()
{
	addVariable(characterName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRemoveFriend::~ChatRemoveFriend()
{
}

//-----------------------------------------------------------------------

