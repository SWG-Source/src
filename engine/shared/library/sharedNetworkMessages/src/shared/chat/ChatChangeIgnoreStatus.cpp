// ChatChangeIgnoreStatus.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatChangeIgnoreStatus.h"

//-----------------------------------------------------------------------

ChatChangeIgnoreStatus::ChatChangeIgnoreStatus(unsigned int q, const ChatAvatarId & c, const ChatAvatarId & _friendName, bool a) :
GameNetworkMessage("ChatChangeIgnoreStatus"),
characterName(c),
friendName(_friendName),
sequence(q),
ignore(a)
{
	addVariable(characterName);
	addVariable(friendName);
	addVariable(sequence);
	addVariable(ignore);
}

//-----------------------------------------------------------------------

ChatChangeIgnoreStatus::ChatChangeIgnoreStatus(Archive::ReadIterator & source) :
GameNetworkMessage("ChatChangeIgnoreStatus"),
characterName(),
friendName(),
sequence(0),
ignore(0)
{
	addVariable(characterName);
	addVariable(friendName);
	addVariable(sequence);
	addVariable(ignore);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatChangeIgnoreStatus::~ChatChangeIgnoreStatus()
{
}

//-----------------------------------------------------------------------

