// ChatBanAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatBanAvatarFromRoom::ChatBanAvatarFromRoom(const unsigned int s, const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatBanAvatarFromRoom"),
avatarId(a),
roomName(r),
sequence(s)
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatBanAvatarFromRoom::ChatBanAvatarFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatBanAvatarFromRoom"),
avatarId(),
roomName()
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatBanAvatarFromRoom::~ChatBanAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

