// ChatOnGetIgnoreList.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnGetIgnoreList.h"

//-----------------------------------------------------------------------

ChatOnGetIgnoreList::ChatOnGetIgnoreList(const NetworkId & c, const std::vector<ChatAvatarId> & ignoreList) :
GameNetworkMessage("ChatOnGetIgnoreList"),
character(c),
ignores()
{
	ignores.set(ignoreList);
	addVariable(character);
	addVariable(ignores);
}

//-----------------------------------------------------------------------

ChatOnGetIgnoreList::ChatOnGetIgnoreList(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnGetIgnoreList"),
character(),
ignores()
{
	addVariable(character);
	addVariable(ignores);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnGetIgnoreList::~ChatOnGetIgnoreList()
{
}

//-----------------------------------------------------------------------

