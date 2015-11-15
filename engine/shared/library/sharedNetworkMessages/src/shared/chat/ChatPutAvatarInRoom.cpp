// ChatPutAvatarInRoom.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ChatPutAvatarInRoom.h"

//-----------------------------------------------------------------------

ChatPutAvatarInRoom::ChatPutAvatarInRoom(const std::string & a, const std::string & r, bool forceCreate, bool createPrivate) :
GameNetworkMessage("ChatPutAvatarInRoom"),
m_avatarName(a),
m_roomName(r),
m_forceCreate(forceCreate),
m_createPrivate(createPrivate)
{
	addVariable(m_avatarName);
	addVariable(m_roomName);
	addVariable(m_forceCreate);
	addVariable(m_createPrivate);
}

//-----------------------------------------------------------------------

ChatPutAvatarInRoom::ChatPutAvatarInRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatPutAvatarInRoom"),
m_avatarName(),
m_roomName(),
m_forceCreate(false),
m_createPrivate(false)
{
	addVariable(m_avatarName);
	addVariable(m_roomName);
	addVariable(m_forceCreate);
	addVariable(m_createPrivate);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatPutAvatarInRoom::~ChatPutAvatarInRoom()
{
}

//-----------------------------------------------------------------------

bool ChatPutAvatarInRoom::getForceCreate() const
{
	return m_forceCreate.get();
}

//-----------------------------------------------------------------------

bool ChatPutAvatarInRoom::getCreatePrivate() const
{
	return m_createPrivate.get();
}

//-----------------------------------------------------------------------
