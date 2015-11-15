// ChatPersistentMessageToServer.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatPersistentMessageToServer.h"

//-----------------------------------------------------------------------


ChatPersistentMessageToServer::ChatPersistentMessageToServer(const unsigned int s, const ChatAvatarId & c, const Unicode::String & subj, const Unicode::String & m, const Unicode::String & o) :
GameNetworkMessage("ChatPersistentMessageToServer"),
message(m.substr(0, ChatPersistentMessageToServerNamespace::MAX_MESSAGE_SIZE)),	
outOfBand(o),
sequence(s),
subject(subj),
toCharacterName(c)
{
	addVariable(message);
	addVariable(outOfBand);
	addVariable(sequence);
	addVariable(subject);
	addVariable(toCharacterName);
}

//-----------------------------------------------------------------------

ChatPersistentMessageToServer::ChatPersistentMessageToServer(Archive::ReadIterator  & source) :
GameNetworkMessage("ChatPersistentMessageToServer"),
message(),
outOfBand(),
subject(),
toCharacterName()
{
	addVariable(message);
	addVariable(outOfBand);
	addVariable(sequence);
	addVariable(subject);
	addVariable(toCharacterName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatPersistentMessageToServer::~ChatPersistentMessageToServer()
{
}

//-----------------------------------------------------------------------

