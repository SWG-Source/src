// ChatRemoveModeratorFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatRemoveModeratorFromRoom.h"

//-----------------------------------------------------------------------

ChatRemoveModeratorFromRoom::ChatRemoveModeratorFromRoom(const unsigned int s, const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatRemoveModeratorFromRoom"),
avatarId(a),
roomName(r),
sequenceId(s)
{
	addVariable(avatarId);
	addVariable(roomName);
	addVariable(sequenceId);
}

//-----------------------------------------------------------------------

ChatRemoveModeratorFromRoom::ChatRemoveModeratorFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatRemoveModeratorFromRoom"),
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

ChatRemoveModeratorFromRoom::~ChatRemoveModeratorFromRoom()
{
}

//-----------------------------------------------------------------------

