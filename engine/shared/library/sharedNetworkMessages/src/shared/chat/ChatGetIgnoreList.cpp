// ChatGetIgnoreList.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatGetIgnoreList.h"

//-----------------------------------------------------------------------

ChatGetIgnoreList::ChatGetIgnoreList(const ChatAvatarId & c) :
GameNetworkMessage("ChatGetIgnoreList"),
characterName(c)
{
	addVariable(characterName);
}

//-----------------------------------------------------------------------

ChatGetIgnoreList::ChatGetIgnoreList(Archive::ReadIterator & source) :
GameNetworkMessage("ChatGetIgnoreList"),
characterName()
{
	addVariable(characterName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatGetIgnoreList::~ChatGetIgnoreList()
{
}

//-----------------------------------------------------------------------

