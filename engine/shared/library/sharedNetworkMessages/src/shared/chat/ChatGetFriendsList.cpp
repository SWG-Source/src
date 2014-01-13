// ChatGetFriendsList.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatGetFriendsList.h"

//-----------------------------------------------------------------------

ChatGetFriendsList::ChatGetFriendsList(const ChatAvatarId & c) :
GameNetworkMessage("ChatGetFriendsList"),
characterName(c)
{
	addVariable(characterName);
}

//-----------------------------------------------------------------------

ChatGetFriendsList::ChatGetFriendsList(Archive::ReadIterator & source) :
GameNetworkMessage("ChatGetFriendsList"),
characterName()
{
	addVariable(characterName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatGetFriendsList::~ChatGetFriendsList()
{
}

//-----------------------------------------------------------------------

