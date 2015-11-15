// ChatCreateRoom.cpp
// Copyright 2000-02, Sony Online Createtainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "ChatCreateRoom.h"

//-----------------------------------------------------------------------

ChatCreateRoom::ChatCreateRoom(const unsigned int s, const std::string & o, const std::string & n, const bool m, const bool p, const std::string & t) :
GameNetworkMessage("ChatCreateRoom"),
isPublic(p),
isModerated(m),
ownerName(o),
roomName(n),
roomTitle(t),
sequence(s)
{
	addVariable(isPublic);
	addVariable(isModerated);
	addVariable(ownerName);
	addVariable(roomName);
	addVariable(roomTitle);
	addVariable(sequence);
}

//-----------------------------------------------------------------------

ChatCreateRoom::ChatCreateRoom(Archive::ReadIterator & source) :
GameNetworkMessage("ChatCreateRoom"),
isPublic(),
isModerated(),
ownerName(),
roomName(),
roomTitle(),
sequence()
{
	addVariable(isPublic);
	addVariable(isModerated);
	addVariable(ownerName);
	addVariable(roomName);
	addVariable(roomTitle);
	addVariable(sequence);
	unpack(source);
}

//-----------------------------------------------------------------------

ChatCreateRoom::~ChatCreateRoom()
{
}

//-----------------------------------------------------------------------

