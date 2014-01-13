// ChatOnRemoveModeratorFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatOnRemoveModeratorFromRoom.h"

//-----------------------------------------------------------------------

ChatOnRemoveModeratorFromRoom::ChatOnRemoveModeratorFromRoom(const unsigned int c, const unsigned int s, const ChatAvatarId & i, const ChatAvatarId &remover, const std::string & r) :
GameNetworkMessage("ChatOnRemoveModeratorFromRoom"),
avatarId(i),
removerId(remover),
resultCode(c),
roomName(r),
sequenceId(s)
{
	addVariable(avatarId);
	addVariable(removerId);
	addVariable(resultCode);
	addVariable(roomName);
	addVariable(sequenceId);
}

//-----------------------------------------------------------------------

ChatOnRemoveModeratorFromRoom::ChatOnRemoveModeratorFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnRemoveModeratorFromRoom"),
avatarId(),
removerId(),
resultCode(),
roomName(),
sequenceId()
{
	addVariable(avatarId);
	addVariable(removerId);
	addVariable(resultCode);
	addVariable(roomName);
	addVariable(sequenceId);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnRemoveModeratorFromRoom::~ChatOnRemoveModeratorFromRoom()
{
}

//-----------------------------------------------------------------------

