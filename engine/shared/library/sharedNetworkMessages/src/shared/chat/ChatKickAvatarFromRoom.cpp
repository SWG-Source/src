// ChatKickAvatarFromRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Roy Fleck

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/ChatAvatarIdArchive.h"
#include "sharedNetworkMessages/ChatKickAvatarFromRoom.h"

//-----------------------------------------------------------------------

ChatKickAvatarFromRoom::ChatKickAvatarFromRoom(
	const ChatAvatarId & avatarId,
	const std::string & roomName
) :
	GameNetworkMessage("ChatKickAvatarFromRoom"),
	m_avatarId(avatarId),
	m_roomName(roomName)
{
	addVariable(m_avatarId);
	addVariable(m_roomName);
}

//-----------------------------------------------------------------------

ChatKickAvatarFromRoom::ChatKickAvatarFromRoom(Archive::ReadIterator & source) :
	GameNetworkMessage("ChatKickAvatarFromRoom"),
	m_avatarId(),
	m_roomName()
{
	addVariable(m_avatarId);
	addVariable(m_roomName);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatKickAvatarFromRoom::~ChatKickAvatarFromRoom()
{
}

//-----------------------------------------------------------------------

