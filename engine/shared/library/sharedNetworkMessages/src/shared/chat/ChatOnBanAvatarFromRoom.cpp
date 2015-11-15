// ChatOnBanAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnBanAvatarFromRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatOnBanAvatarFromRoom::ChatOnBanAvatarFromRoom(const unsigned int s, const unsigned int r, const std::string &room, const ChatAvatarId &bannerId, const ChatAvatarId &banneeId) :
GameNetworkMessage("ChatOnBanAvatarFromRoom"),
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

ChatOnBanAvatarFromRoom::ChatOnBanAvatarFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnBanAvatarFromRoom"),
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

ChatOnBanAvatarFromRoom::~ChatOnBanAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

