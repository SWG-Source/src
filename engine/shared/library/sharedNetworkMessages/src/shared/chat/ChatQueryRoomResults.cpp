// ChatQueryRoomResults.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatQueryRoomResults.h"

//-----------------------------------------------------------------------

ChatQueryRoomResults::ChatQueryRoomResults(const unsigned int s, const ChatRoomData & n, const std::vector<ChatAvatarId> & a, const std::vector<ChatAvatarId> & m, const std::vector<ChatAvatarId> & i, const std::vector<ChatAvatarId> & b) :
GameNetworkMessage("ChatQueryRoomResults"),
avatars(),
invitees(),
moderators(),
banned(),
roomData(n),
sequence(s)
{
	avatars.set(a);
	invitees.set(i);
	moderators.set(m);
	banned.set(b);
	addVariable(avatars);
	addVariable(invitees);
	addVariable(moderators);
	addVariable(banned);
	addVariable(sequence);
	addVariable(roomData);
}

//-----------------------------------------------------------------------

ChatQueryRoomResults::ChatQueryRoomResults(Archive::ReadIterator & source) :
GameNetworkMessage("ChatQueryRoomResults"),
avatars(),
invitees(),
moderators(),
banned(),
roomData(),
sequence()
{
	addVariable(avatars);
	addVariable(invitees);
	addVariable(moderators);
	addVariable(banned);
	addVariable(sequence);
	addVariable(roomData);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatQueryRoomResults::~ChatQueryRoomResults()
{
}

//----------------------------------------------------------------------

const std::vector<ChatAvatarId> & ChatQueryRoomResults::getBanned () const
{
	return banned.get();
}

//-----------------------------------------------------------------------

