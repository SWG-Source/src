// ChatOnUninviteFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatOnUninviteFromRoom.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"

//-----------------------------------------------------------------------

ChatOnUninviteFromRoom::ChatOnUninviteFromRoom(const unsigned int s, const unsigned int r, const std::string &room, const ChatAvatarId &invitorId, const ChatAvatarId &inviteeId) :
GameNetworkMessage("ChatOnUninviteFromRoom"),
roomName(room),
invitor(invitorId),
invitee(inviteeId),
result(r),
sequence(s)
{
	addVariable(roomName);
	addVariable(invitor);
	addVariable(invitee);
	addVariable(result);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatOnUninviteFromRoom::ChatOnUninviteFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatOnUninviteFromRoom"),
roomName(),
invitor(),
invitee(),
result(),
sequence()
{
	addVariable(roomName);
	addVariable(invitor);
	addVariable(invitee);
	addVariable(result);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatOnUninviteFromRoom::~ChatOnUninviteFromRoom()
{
}

//-----------------------------------------------------------------------

