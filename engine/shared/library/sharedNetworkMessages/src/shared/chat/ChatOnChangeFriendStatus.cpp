// ChatOnChangeFriendStatus.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Matt Sivertson

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatOnChangeFriendStatus.h"

//-----------------------------------------------------------------------

ChatOnChangeFriendStatus::ChatOnChangeFriendStatus(unsigned int q, const NetworkId & c, const ChatAvatarId & _friendName, bool a, unsigned int result) :
GameNetworkMessage("ChatOnChangeFriendStatus"),
character(c),
friendName(_friendName),
sequence(q),
add(a),
resultCode(result)
{
	addVariable(character);
	addVariable(friendName);
	addVariable(sequence);
	addVariable(add);
	addVariable(resultCode);
}

//-----------------------------------------------------------------------

ChatOnChangeFriendStatus::ChatOnChangeFriendStatus(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnChangeFriendStatus"),
character(),
friendName(),
sequence(0),
add(0),
resultCode(0)
{
	addVariable(character);
	addVariable(friendName);
	addVariable(sequence);
	addVariable(add);
	addVariable(resultCode);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnChangeFriendStatus::~ChatOnChangeFriendStatus()
{
}

//-----------------------------------------------------------------------

