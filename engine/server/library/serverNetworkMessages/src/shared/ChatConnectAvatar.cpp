// ChatConnectAvatar.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "ChatConnectAvatar.h"

//-----------------------------------------------------------------------

ChatConnectAvatar::ChatConnectAvatar(const std::string & n, const NetworkId & i, const unsigned int s, bool secure, bool subscribed) :
GameNetworkMessage("ChatConnectAvatar"),
characterId(i),
characterName(n),
stationId(s),
isSecure(secure),
isSubscribed(subscribed)
{
	addVariable(characterId);
	addVariable(characterName);
	addVariable(stationId);
	addVariable(isSecure);
	addVariable(isSubscribed);
}

//-----------------------------------------------------------------------

ChatConnectAvatar::ChatConnectAvatar(Archive::ReadIterator & source) :
GameNetworkMessage("ChatConnectAvatar"),
characterId(),
characterName(),
stationId(),
isSecure(false),
isSubscribed(false)
{
	addVariable(characterId);
	addVariable(characterName);
	addVariable(stationId);
	addVariable(isSecure);
	addVariable(isSubscribed);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatConnectAvatar::~ChatConnectAvatar()
{
}

//-----------------------------------------------------------------------
