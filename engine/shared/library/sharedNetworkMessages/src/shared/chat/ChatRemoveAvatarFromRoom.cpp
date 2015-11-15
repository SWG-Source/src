// ChatRemoveAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatRemoveAvatarFromRoom.h"

//-----------------------------------------------------------------------

ChatRemoveAvatarFromRoom::ChatRemoveAvatarFromRoom(const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatRemoveAvatarFromRoom"),
avatarId(a),
roomName(r)
{
	addVariable(avatarId);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatRemoveAvatarFromRoom::ChatRemoveAvatarFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRemoveAvatarFromRoom"),
avatarId(),
roomName()
{
	addVariable(avatarId);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatRemoveAvatarFromRoom::~ChatRemoveAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

