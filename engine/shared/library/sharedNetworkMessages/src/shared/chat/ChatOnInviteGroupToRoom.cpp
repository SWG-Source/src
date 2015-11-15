// ChatOnInviteToRoom.cpp
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnInviteGroupToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatOnInviteGroupToRoom::ChatOnInviteGroupToRoom(const unsigned int r, const std::string &room, const ChatAvatarId &invitorId, const ChatAvatarId &inviteeId) :
GameNetworkMessage("ChatOnInviteGroupToRoom"),
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

ChatOnInviteGroupToRoom::ChatOnInviteGroupToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnInviteGroupToRoom"),
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

ChatOnInviteGroupToRoom::~ChatOnInviteGroupToRoom()
{
}

//-----------------------------------------------------------------------

