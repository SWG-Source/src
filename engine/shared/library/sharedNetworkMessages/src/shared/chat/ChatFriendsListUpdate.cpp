// ChatFriendsListUpdate.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatFriendsListUpdate.h"

//-----------------------------------------------------------------------

ChatFriendsListUpdate::ChatFriendsListUpdate(const ChatAvatarId & c, const bool o) :
GameNetworkMessage("ChatFriendsListUpdate"),
characterName(c),
isOnline(o)
{
	addVariable(characterName);
	addVariable(isOnline);
}

//-----------------------------------------------------------------------

ChatFriendsListUpdate::ChatFriendsListUpdate(Archive::ReadIterator & source) :
GameNetworkMessage("ChatFriendsListUpdate"),
characterName(),
isOnline()
{
	addVariable(characterName);
	addVariable(isOnline);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatFriendsListUpdate::~ChatFriendsListUpdate()
{
}

//-----------------------------------------------------------------------

