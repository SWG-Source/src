// ChatOnReceiveRoomInvitation.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatOnReceiveRoomInvitation.h"


//-----------------------------------------------------------------------

ChatOnReceiveRoomInvitation::ChatOnReceiveRoomInvitation(const std::string & r, const ChatAvatarId & id) :
GameNetworkMessage("ChatOnReceiveRoomInvitation"),
invitorAvatar(id),
roomName(r)
{
	addVariable(invitorAvatar);
	addVariable(roomName);
}

//-----------------------------------------------------------------------

ChatOnReceiveRoomInvitation::ChatOnReceiveRoomInvitation(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnReceiveRoomInvitation"),
invitorAvatar(),
roomName()
{
	addVariable(invitorAvatar);
	addVariable(roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnReceiveRoomInvitation::~ChatOnReceiveRoomInvitation()
{
}

//-----------------------------------------------------------------------

