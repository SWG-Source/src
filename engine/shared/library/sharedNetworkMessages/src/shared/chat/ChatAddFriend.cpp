// ChatAddFriend.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatAddFriend.h"

//-----------------------------------------------------------------------

ChatAddFriend::ChatAddFriend(const unsigned int q, const ChatAvatarId & c) :
GameNetworkMessage("ChatAddFriend"),
characterName(c),
sequence(q)
{
	addVariable(characterName);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatAddFriend::ChatAddFriend(Archive::ReadIterator & source) :
GameNetworkMessage("ChatAddFriend"),
characterName(),
sequence()
{
	addVariable(characterName);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatAddFriend::~ChatAddFriend()
{
}

//-----------------------------------------------------------------------

