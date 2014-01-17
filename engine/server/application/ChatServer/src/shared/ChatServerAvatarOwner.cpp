// ChatServerAvatarOwner.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "FirstChatServer.h"
#include "ChatServerAvatarOwner.h"
#include "ConnectionServerConnection.h"

//-----------------------------------------------------------------------

ChatServerAvatarOwner::ChatServerAvatarOwner(ConnectionServerConnection * c, const NetworkId & n) :
networkId(n),
playerConnection(c)
{
	c->addAvatar(this);
}

//-----------------------------------------------------------------------

ChatServerAvatarOwner::~ChatServerAvatarOwner()
{
	if(playerConnection)
	{
		playerConnection->removeAvatar(this);
	}
	
	playerConnection = 0;
}

//-----------------------------------------------------------------------

void ChatServerAvatarOwner::setPlayerConnection(ConnectionServerConnection * c)
{
	playerConnection = c;
}

//-----------------------------------------------------------------------

