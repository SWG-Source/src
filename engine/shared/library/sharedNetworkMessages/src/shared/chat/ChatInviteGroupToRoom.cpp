// ChatInviteGroupToRoom.cpp
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatInviteGroupToRoom::ChatInviteGroupToRoom(const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatInviteGroupToRoom"),
avatarId(a),
roomName(r)
{
	addVariable(avatarId);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatInviteGroupToRoom::ChatInviteGroupToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatInviteGroupToRoom"),
avatarId(),
roomName()
{
	addVariable(avatarId);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatInviteGroupToRoom::~ChatInviteGroupToRoom()
{
}

//-----------------------------------------------------------------------

