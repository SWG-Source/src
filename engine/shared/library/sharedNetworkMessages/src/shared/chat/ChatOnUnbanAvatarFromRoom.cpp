// ChatOnUnbanAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnUnbanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatOnUnbanAvatarFromRoom::ChatOnUnbanAvatarFromRoom(const unsigned int s, const unsigned int r, const std::string &room, const ChatAvatarId &bannerId, const ChatAvatarId &banneeId) :
GameNetworkMessage("ChatOnUnbanAvatarFromRoom"),
roomName(room),
banner(bannerId),
bannee(banneeId),
result(r),
sequence(s)
{
	addVariable(roomName);
	addVariable(banner);
	addVariable(bannee);
	addVariable(result);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnUnbanAvatarFromRoom::ChatOnUnbanAvatarFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnUnbanAvatarFromRoom"),
roomName(),
banner(),
bannee(),
result(),
sequence()
{
	addVariable(roomName);
	addVariable(banner);
	addVariable(bannee);
	addVariable(result);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnUnbanAvatarFromRoom::~ChatOnUnbanAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

