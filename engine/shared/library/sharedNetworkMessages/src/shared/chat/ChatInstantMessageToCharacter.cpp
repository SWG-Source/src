// ChatInstantMessageToCharacter.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatInstantMessageToCharacter.h"

//-----------------------------------------------------------------------

ChatInstantMessageToCharacter::ChatInstantMessageToCharacter(const unsigned int s, const ChatAvatarId & c, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatInstantMessageToCharacter"),
characterName(c),
message(m),
outOfBand(o),
sequence(s)
{
	addVariable(characterName);
	addVariable(message);
	addVariable(outOfBand);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatInstantMessageToCharacter::ChatInstantMessageToCharacter(Archive::ReadIterator & source) :
GameNetworkMessage("ChatInstantMessageToCharacter"),
characterName(),
message(),
outOfBand(),
sequence()
{
	addVariable(characterName);
	addVariable(message);
	addVariable(outOfBand);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatInstantMessageToCharacter::~ChatInstantMessageToCharacter()
{
}

//-----------------------------------------------------------------------

