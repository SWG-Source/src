// ChatOnConnectAvatar.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_ChatOnConnectAvatar_H
#define	_INCLUDED_ChatOnConnectAvatar_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ChatOnConnectAvatar : public GameNetworkMessage
{
public:
	ChatOnConnectAvatar();
	ChatOnConnectAvatar(Archive::ReadIterator & source);
	~ChatOnConnectAvatar();

private:
	ChatOnConnectAvatar & operator = (const ChatOnConnectAvatar & rhs);
	ChatOnConnectAvatar(const ChatOnConnectAvatar & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatOnConnectAvatar_H

