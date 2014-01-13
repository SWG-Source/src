// ChatUninviteFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatUninviteFromRoom.h"

//-----------------------------------------------------------------------

ChatUninviteFromRoom::ChatUninviteFromRoom(const unsigned int s, const ChatAvatarId & a, const std::string & r) :
GameNetworkMessage("ChatUninviteFromRoom"),
avatar(a),
roomName(r),
sequence(s)
{
	addVariable(avatar);
	addVariable(roomName);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatUninviteFromRoom::ChatUninviteFromRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatUninviteFromRoom"),
avatar(),
roomName(),
sequence()
{
	addVariable(avatar);
	addVariable(roomName);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatUninviteFromRoom::~ChatUninviteFromRoom()
{
}

//-----------------------------------------------------------------------

