// ChatOnInviteToRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnInviteToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatOnInviteToRoom::ChatOnInviteToRoom(const unsigned int r, const std::string &room, const ChatAvatarId &invitorId, const ChatAvatarId &inviteeId) :
GameNetworkMessage("ChatOnInviteToRoom"),
roomName(room),
invitor(invitorId),
invitee(inviteeId),
result(r)
{
	addVariable(roomName);
	addVariable(invitor);
	addVariable(invitee);
	addVariable(result);
}

//-----------------------------------------------------------------------

ChatOnInviteToRoom::ChatOnInviteToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnInviteToRoom"),
roomName(),
invitor(),
invitee(),
result()
{
	addVariable(roomName);
	addVariable(invitor);
	addVariable(invitee);
	addVariable(result);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnInviteToRoom::~ChatOnInviteToRoom()
{
}

//-----------------------------------------------------------------------

