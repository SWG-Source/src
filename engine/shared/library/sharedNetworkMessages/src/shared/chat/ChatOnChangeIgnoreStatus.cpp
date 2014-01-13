// ChatOnChangeIgnoreStatus.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnChangeIgnoreStatus.h"

//-----------------------------------------------------------------------

ChatOnChangeIgnoreStatus::ChatOnChangeIgnoreStatus(unsigned int q, const NetworkId & c, const ChatAvatarId & _ignoreName, bool a, unsigned int result) :
GameNetworkMessage("ChatOnChangeIgnoreStatus"),
character(c),
ignoreName(_ignoreName),
sequence(q),
ignore(a),
resultCode(result)
{
	addVariable(character);
	addVariable(ignoreName);
	addVariable(sequence);
	addVariable(ignore);
	addVariable(resultCode);
}

//-----------------------------------------------------------------------

ChatOnChangeIgnoreStatus::ChatOnChangeIgnoreStatus(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnChangeIgnoreStatus"),
character(),
ignoreName(),
sequence(0),
ignore(0),
resultCode(0)
{
	addVariable(character);
	addVariable(ignoreName);
	addVariable(sequence);
	addVariable(ignore);
	addVariable(resultCode);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnChangeIgnoreStatus::~ChatOnChangeIgnoreStatus()
{
}

//-----------------------------------------------------------------------

