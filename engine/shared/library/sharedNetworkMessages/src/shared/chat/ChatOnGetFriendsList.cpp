// ChatOnGetFriendsList.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnGetFriendsList.h"

//-----------------------------------------------------------------------

ChatOnGetFriendsList::ChatOnGetFriendsList(const NetworkId & c, const std::vector<ChatAvatarId> & friendsList) :
GameNetworkMessage("ChatOnGetFriendsList"),
character(c),
friends()
{
	friends.set(friendsList);
	addVariable(character);
	addVariable(friends);
}

//-----------------------------------------------------------------------

ChatOnGetFriendsList::ChatOnGetFriendsList(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnGetFriendsList"),
character(),
friends()
{
	addVariable(character);
	addVariable(friends);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnGetFriendsList::~ChatOnGetFriendsList()
{
}

//-----------------------------------------------------------------------

