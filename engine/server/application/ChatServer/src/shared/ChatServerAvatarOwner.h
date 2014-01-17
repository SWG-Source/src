// ChatServerAvatarOwner.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_ChatServerAvatarOwner_H
#define	_INCLUDED_ChatServerAvatarOwner_H

//-----------------------------------------------------------------------

#include "ChatAPI/ChatAvatar.h"
#include "sharedFoundation/NetworkId.h"

class ConnectionServerConnection;

//-----------------------------------------------------------------------

class ChatServerAvatarOwner
{
public:
	explicit ChatServerAvatarOwner(ConnectionServerConnection * playerConnection, const NetworkId & networkId);
	~ChatServerAvatarOwner();

	const NetworkId &                   getNetworkId         () const;
	ConnectionServerConnection *        getPlayerConnection  ();
	void                                setPlayerConnection  (ConnectionServerConnection *);
	
private:
	ChatServerAvatarOwner & operator = (const ChatServerAvatarOwner & rhs);
	ChatServerAvatarOwner(const ChatServerAvatarOwner & source);
	NetworkId                          networkId;
	ConnectionServerConnection * playerConnection;
}; //lint !e1712 default constructor not defined for class

//-----------------------------------------------------------------------

inline const NetworkId & ChatServerAvatarOwner::getNetworkId() const
{
	return networkId;
}

//-----------------------------------------------------------------------

inline ConnectionServerConnection * ChatServerAvatarOwner::getPlayerConnection()
{
	return playerConnection;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ChatServerAvatarOwner_H
