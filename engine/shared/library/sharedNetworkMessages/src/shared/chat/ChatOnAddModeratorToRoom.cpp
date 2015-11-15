// ChatOnAddModeratorToRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnAddModeratorToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatOnAddModeratorToRoom::ChatOnAddModeratorToRoom(const unsigned int c, const unsigned int s, const ChatAvatarId & a, const ChatAvatarId &granter, const std::string & r) :
GameNetworkMessage("ChatOnAddModeratorToRoom"),
avatarId(a),
granterId(granter),
resultCode(c),
roomName(r),
sequenceId(s)
{
	addVariable(avatarId);
	addVariable(granterId);
	addVariable(resultCode);
	addVariable(roomName);
	addVariable(sequenceId);
}

//-----------------------------------------------------------------------

ChatOnAddModeratorToRoom::ChatOnAddModeratorToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnAddModeratorToRoom"),
avatarId(),
granterId(),
resultCode(),
roomName(),
sequenceId()
{
	addVariable(avatarId);
	addVariable(granterId);
	addVariable(resultCode);
	addVariable(roomName);
	addVariable(sequenceId);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnAddModeratorToRoom::~ChatOnAddModeratorToRoom()
{
}

//-----------------------------------------------------------------------
