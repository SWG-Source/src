// ChatOnKickAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatOnKickAvatarFromRoom.h"

//-----------------------------------------------------------------------

ChatOnKickAvatarFromRoom::ChatOnKickAvatarFromRoom(const unsigned int c, const ChatAvatarId & i, const ChatAvatarId &remover, const std::string & r) :
GameNetworkMessage("ChatOnKickAvatarFromRoom"),
avatarId(i),
removerId(remover),
resultCode(c),
roomName(r)
{
	addVariable(avatarId);
	addVariable(removerId);
	addVariable(resultCode);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatOnKickAvatarFromRoom::ChatOnKickAvatarFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnKickAvatarFromRoom"),
avatarId(),
removerId(),
resultCode(),
roomName()
{
	addVariable(avatarId);
	addVariable(removerId);
	addVariable(resultCode);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnKickAvatarFromRoom::~ChatOnKickAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

