// ChatUnbanAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatUnbanAvatarFromRoom::ChatUnbanAvatarFromRoom(const unsigned int s, const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatUnbanAvatarFromRoom"),
avatarId(a),
roomName(r),
sequence(s)
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatUnbanAvatarFromRoom::ChatUnbanAvatarFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatUnbanAvatarFromRoom"),
avatarId(),
roomName()
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatUnbanAvatarFromRoom::~ChatUnbanAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

