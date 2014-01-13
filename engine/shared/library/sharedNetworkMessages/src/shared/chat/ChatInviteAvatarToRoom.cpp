// ChatInviteAvatarToRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatInviteAvatarToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatInviteAvatarToRoom::ChatInviteAvatarToRoom(const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatInviteAvatarToRoom"),
avatarId(a),
roomName(r)
{
	addVariable(avatarId);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatInviteAvatarToRoom::ChatInviteAvatarToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatInviteAvatarToRoom"),
avatarId(),
roomName()
{
	addVariable(avatarId);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatInviteAvatarToRoom::~ChatInviteAvatarToRoom()
{
}

//-----------------------------------------------------------------------

