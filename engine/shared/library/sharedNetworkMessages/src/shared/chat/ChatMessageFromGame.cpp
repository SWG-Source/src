// ChatMessageFromGame.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatMessageFromGame.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

ChatMessageFromGame::ChatMessageFromGame(enum MessageTypes mt, const std::string & f, const std::string & t, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatMessageFromGame"),
from(f),
message(m),
messageType(static_cast<unsigned char>(mt)),
oob(o),
room(""),
subject(Unicode::narrowToWide("")),
to(t)
{
	addVariable(from);
	addVariable(message);
	addVariable(messageType);
	addVariable(oob);
	addVariable(room);
	addVariable(subject);
	addVariable(to);
}

//-----------------------------------------------------------------------

ChatMessageFromGame::ChatMessageFromGame(enum MessageTypes mt, const std::string & f, const std::string & t, const Unicode::String & m, const Unicode::String & o, const Unicode::String & s) :
GameNetworkMessage("ChatMessageFromGame"),
from(f),
message(m),
messageType(static_cast<unsigned char>(mt)),
oob(o),
room(""),
subject(s),
to(t)
{
	addVariable(from);
	addVariable(message);
	addVariable(messageType);
	addVariable(oob);
	addVariable(room);
	addVariable(subject);
	addVariable(to);
}

//-----------------------------------------------------------------------

ChatMessageFromGame::ChatMessageFromGame(enum MessageTypes mt, const std::string & f, const Unicode::String & m, const Unicode::String & o, const std::string & r) :
GameNetworkMessage("ChatMessageFromGame"),
from(f),
message(m),
messageType(static_cast<unsigned char>(mt)),
oob(o),
room(r),
subject(Unicode::narrowToWide("")),
to()
{
	addVariable(from);
	addVariable(message);
	addVariable(messageType);
	addVariable(oob);
	addVariable(room);
	addVariable(subject);
	addVariable(to);
}

//-----------------------------------------------------------------------

ChatMessageFromGame::ChatMessageFromGame(Archive::ReadIterator & source) :
GameNetworkMessage("ChatMessageFromGame"),
from(),
message(),
messageType(INSTANT),
oob(),
room(),
subject(),
to()
{
	addVariable(from);
	addVariable(message);
	addVariable(messageType);
	addVariable(oob);
	addVariable(room);
	addVariable(subject);
	addVariable(to);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatMessageFromGame::~ChatMessageFromGame()
{
}

//-----------------------------------------------------------------------

