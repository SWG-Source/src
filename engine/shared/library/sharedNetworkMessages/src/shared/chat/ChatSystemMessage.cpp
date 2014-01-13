// ChatSystemMessage.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatSystemMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

ChatSystemMessage::ChatSystemMessage(const unsigned char f, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatSystemMessage"),
flags(f),
message(m),
outOfBand(o)
{
	addVariable(flags);
	addVariable(message);
	addVariable(outOfBand);
}

//-----------------------------------------------------------------------

ChatSystemMessage::ChatSystemMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ChatSystemMessage"),
flags(),
message(),
outOfBand()
{
	addVariable(flags);
	addVariable(message);
	addVariable(outOfBand);
	unpack(source);	
}

//-----------------------------------------------------------------------

ChatSystemMessage::~ChatSystemMessage()
{
}

//-----------------------------------------------------------------------

