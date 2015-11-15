// ChatAddModeratorToRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatAddModeratorToRoom::ChatAddModeratorToRoom(const unsigned int s, const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatAddModeratorToRoom"),
avatarId(a),
roomName(r),
sequenceId(s)
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequenceId);
}

//-----------------------------------------------------------------------

ChatAddModeratorToRoom::ChatAddModeratorToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatAddModeratorToRoom"),
avatarId(),
roomName(),
sequenceId(0)
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequenceId);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatAddModeratorToRoom::~ChatAddModeratorToRoom()
{
}

//-----------------------------------------------------------------------

