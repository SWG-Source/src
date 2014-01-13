// ChatChangeFriendStatus.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatChangeFriendStatus.h"

//-----------------------------------------------------------------------

ChatChangeFriendStatus::ChatChangeFriendStatus(unsigned int q, const ChatAvatarId & c, const ChatAvatarId & _friendName, bool a) :
GameNetworkMessage("ChatChangeFriendStatus"),
characterName(c),
friendName(_friendName),
sequence(q),
add(a)
{
	addVariable(characterName);
	addVariable(friendName);
	addVariable(sequence);
	addVariable(add);
}

//-----------------------------------------------------------------------

ChatChangeFriendStatus::ChatChangeFriendStatus(Archive::ReadIterator & source) :
GameNetworkMessage("ChatChangeFriendStatus"),
characterName(),
friendName(),
sequence(0),
add(0)
{
	addVariable(characterName);
	addVariable(friendName);
	addVariable(sequence);
	addVariable(add);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatChangeFriendStatus::~ChatChangeFriendStatus()
{
}

//-----------------------------------------------------------------------

