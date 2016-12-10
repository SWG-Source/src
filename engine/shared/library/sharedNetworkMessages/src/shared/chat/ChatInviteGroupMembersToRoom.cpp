// ChatInviteGroupMembersToRoom.cpp
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: RoyFleck

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatInviteGroupMembersToRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatInviteGroupMembersToRoom::ChatInviteGroupMembersToRoom(const NetworkId & i, const ChatAvatarId & g, const std::string & r, const std::vector<NetworkId> & m) :
GameNetworkMessage("ChatInviteGroupMembersToRoom"),
invitorNetworkId(i),
groupLeaderId(g),
roomName(r),
invitedMembers()
{
	invitedMembers.set(m);

	addVariable(invitorNetworkId);
	addVariable(groupLeaderId);
	addVariable(roomName);
	addVariable(invitedMembers);
}

//-----------------------------------------------------------------------

ChatInviteGroupMembersToRoom::ChatInviteGroupMembersToRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatInviteGroupMembersToRoom"),
invitorNetworkId(),
groupLeaderId(),
roomName(),
invitedMembers()
{
	addVariable(invitorNetworkId);
	addVariable(groupLeaderId);
	addVariable(roomName);
	addVariable(invitedMembers);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatInviteGroupMembersToRoom::~ChatInviteGroupMembersToRoom()
{
}

//-----------------------------------------------------------------------

const std::vector<NetworkId> & ChatInviteGroupMembersToRoom::getInvitedMembers() const
{
	return invitedMembers.get();
}

//-----------------------------------------------------------------------
